#include "h/saveuser.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <iterator>  // Needed for std::istreambuf_iterator

// Define the extern variable.
std::string super_user = "";
const std::string masked_file = "config.sys"; // Obscure file name

void saveUserKeyIfWelcome(const std::string& response) {
    const std::string keyword = "welcome";
    size_t pos = response.find(keyword);
    if (pos != std::string::npos) {
        // Extract text after "welcome"
        super_user = response.substr(pos + keyword.size());
        std::ofstream ofs(masked_file, std::ios::binary);
        if (!ofs) {
            throw std::runtime_error("Unable to open file: " + masked_file);
        }
        ofs.write(super_user.data(), super_user.size());
        ofs.flush(); // Ensure data is written immediately
    }
}

std::string loadSuperUser() {
    std::ifstream ifs(masked_file, std::ios::binary);
    if (ifs) {
        super_user.assign((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
        // You can also check if the file is empty and return "not exists" if needed.
        return "exists";
    } else {
        super_user.clear();
        return "not exists";
    }
}
