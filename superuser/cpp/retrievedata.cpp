#include "../h/retrievedata.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>
#include <thread>
#include "../h/events.h"

using json = nlohmann::json;

// Helper function to convert std::string to std::wstring
std::wstring stringToWstring(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

// Callback function to capture libcurl's response.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Handles connection errors and delays for 5 seconds without blocking UI.
void HandleError(HWND hwnd, const std::wstring& displayText, COLORREF color)
{
    validateclick(hwnd, displayText.c_str(), color); // Display error message with color

    // Run sleep in a separate thread to avoid blocking the UI
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }).detach();
}

// Sets UI elements safely
void setElement(HWND hwnd, const std::string& text, const std::string& filename) {
    SetWindowTextW(hTextView, L""); // Clear text in RichEdit
    SetWindowTextW(hFileValue, L""); // Clear filename in Static control
    SetWindowTextW(hTextView, stringToWstring(text).c_str()); // Set text in RichEdit
    SetWindowTextW(hFileValue, stringToWstring(filename).c_str()); // Set filename in Static control
}

void RetrieveTextAndFilename(HWND hwnd, const std::string& userkey)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        HandleError(hwnd, L"Check your internet!", RGB(255, 0, 0)); // Red color
        return;
    }

    // The endpoint URL (adjust as needed).
    std::string url = "https://syntaxshelf.com/retrieve_json.php";

    // Construct JSON payload with the parameter.
    json j;
    j["userkey"] = userkey;
    std::string postData = j.dump();

    // Set up the headers for JSON.
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string contentLengthHeader = "Content-Length: " + std::to_string(postData.size());
    headers = curl_slist_append(headers, contentLengthHeader.c_str());

    // Prepare to capture the response.
    std::string readBuffer;

    // Set curl options.
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L); // POST request.
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        HandleError(hwnd, L"Check your internet!", RGB(255, 0, 0)); // Red color
    } else {
        try {
            // Parse the response as JSON.
            json respJson = json::parse(readBuffer);
            // Check if the response contains an "error" key.
            if (respJson.contains("error")) {
               HandleError(hwnd, L"Node doesn't exist!", RGB(178, 190, 181)); // Ash color
            } else {
                std::string text = respJson.value("text", "");
                std::string filename = respJson.value("filename", "");
                setElement(hwnd, text, filename);
            }
        } catch (const std::exception& e) {
            HandleError(hwnd, L"Check your internet!", RGB(255, 0, 0)); // Red color
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
