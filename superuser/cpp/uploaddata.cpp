#include <curl/curl.h>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../h/pressorfile.h"
#include <vector>
#include <windows.h>

// Helper function: Converts std::wstring to std::string (assumes ASCII)
static std::string ConvertWStringToString(const std::wstring &wstr) {
    return std::string(wstr.begin(), wstr.end());
}

// Helper function: Converts std::string to std::wstring
static std::wstring ConvertStringToWString(const std::string &str) {
    return std::wstring(str.begin(), str.end());
}

// Helper function: Reads the contents of a file into a string (in binary mode)
static std::string ReadFileContentsAsString(const std::wstring &filePath) {
    std::ifstream file(ConvertWStringToString(filePath), std::ios::binary); // Open file in binary mode
    if (!file) {
        return "";  // Return empty string if the file cannot be opened
    }

    // Get the size of the file
    file.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    // Read the entire file content into a buffer (binary data)
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    // Typecast the buffer to a string (raw binary content)
    return std::string(buffer.begin(), buffer.end());
}

// Callback function to capture the response from the server
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        // Handle memory allocation error
        return 0;
    }
    return newLength;
}

void UploadData(HWND hwnd, const std::wstring &userkey, const std::wstring &text, const std::wstring &filePath) {
    // Read file contents as a string
    std::string fileContent = ReadFileContentsAsString(filePath);
    if (fileContent.empty()) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return;  // Exit the function if the file cannot be read
    }

    // Build the JSON payload using nlohmann::json.
    nlohmann::json j;
    j["userkey"] = ConvertWStringToString(userkey);
    j["text"]    = ConvertWStringToString(text);
    j["filename"] = ConvertWStringToString(filePath);
    j["file"] = fileContent;  // Adding the raw binary content as string
    std::string jsonPayload = j.dump();

    // Initialize libcurl
    CURL *curl = curl_easy_init();
    if (!curl) {
        validateclickzWithTimer(hwnd, L"Check your internet!", RGB(255, 0, 0), 3000);
        return;  // Exit the function if libcurl initialization fails
    }

    // Set up the URL
    std::string url = "https://syntaxshelf.com/upload_json.php";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set up the POST request
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

    // Set up the headers
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Capture the server response
    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Handle the response
    if (res == CURLE_OK) {
        try {
            // Parse the JSON response
            nlohmann::json jsonResponse = nlohmann::json::parse(responseData);

            if (jsonResponse.contains("status") && jsonResponse["status"] == "success") {
                // Success response
                std::wstring successMessage = ConvertStringToWString(jsonResponse["message"]);
                validateclickzWithTimer(hwnd, successMessage.c_str(), RGB(0, 255, 0), 3000); // Green color
            } else if (jsonResponse.contains("error")) {
                // Error response
                std::wstring errorMessage = ConvertStringToWString(jsonResponse["error"]);
                validateclickzWithTimer(hwnd, errorMessage.c_str(), RGB(255, 0, 0), 3000); // Red color
            } else {
                // Invalid response
                validateclickzWithTimer(hwnd, L"Error updating file!", RGB(255, 0, 0), 3000);
            }
        } catch (const std::exception& e) {
            // JSON parsing error
           validateclickzWithTimer(hwnd, L"Error updating file!", RGB(255, 0, 0), 3000);
        }
    } else {
        // libcurl request failed
        validateclickzWithTimer(hwnd, L"Error updating file!", RGB(255, 0, 0), 3000);
    }
}