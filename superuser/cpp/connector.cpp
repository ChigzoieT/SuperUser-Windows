#include <curl/curl.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <algorithm>

// Use nlohmann::json namespace alias
using json = nlohmann::json;

// Helper function to remove null characters from a string.
std::string removeNullChars(const std::string& str) {
    std::string cleanStr(str);
    cleanStr.erase(std::remove(cleanStr.begin(), cleanStr.end(), '\0'), cleanStr.end());
    return cleanStr;
}

// Callback function to collect the response from curl.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    std::string* response = static_cast<std::string*>(userp);
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string sendToPHP(const std::string& userkey, const std::string& password,
                      const std::string& state, const std::string& cmptext)
{
    // Clean input strings.
    std::string cleanUserkey = removeNullChars(userkey);
    std::string cleanPassword = removeNullChars(password);
    std::string cleanState = removeNullChars(state);
    std::string cleanCmptext = removeNullChars(cmptext);

    // Construct JSON payload.
    json jsonData = {
        {"userkey", cleanUserkey},
        {"password", cleanPassword},
        {"state", cleanState},
        {"cmptext", cleanCmptext}
    };

    std::string postData = jsonData.dump();  // Convert JSON to string.
    std::cout << "Sending JSON Data:\n" << postData << std::endl;

    CURL *curl;
    CURLcode res;
    std::string response;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        // Set the URL to your PHP script.
        curl_easy_setopt(curl, CURLOPT_URL, "https://syntaxshelf.com/createuser.php");

        // Set the POST data.
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        // Set up headers.
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string contentLengthHeader = "Content-Length: " + std::to_string(postData.size());
        headers = curl_slist_append(headers, contentLengthHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set callback to capture response.
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Optionally, disable SSL verification (not recommended for production)
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " 
                      << curl_easy_strerror(res) << std::endl;
            response = "Check your internet connection";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        response = "Check your internet connection";
    }
    curl_global_cleanup();

    return response;
}
