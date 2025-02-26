#include "../h/downloaddata.h"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

#pragma comment(lib, "wininet.lib")

// Define the endpoint base and default server.
static const std::wstring ENDPOINT_BASE = L"/retrieve_json.php?userkey=";
static const std::wstring DEFAULT_SERVER = L"localhost";

// Helper function: Performs an HTTP GET request using the given resource URL.
// Returns the response as a wide string.
static std::wstring HttpGetRequest(const std::wstring& resource) {
    std::wstring response;

    // Open an internet session.
    HINTERNET hInternet = InternetOpen(L"WinINet Client/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet)
        return response;

    // Connect to the default server.
    HINTERNET hConnect = InternetConnect(hInternet, DEFAULT_SERVER.c_str(), INTERNET_DEFAULT_HTTP_PORT,
                                          NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return response;
    }

    // Open an HTTP GET request with the resource (includes the userkey).
    HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", resource.c_str(), NULL, NULL, NULL, 0, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return response;
    }

    // Send the request.
    BOOL bRequestSent = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
    if (!bRequestSent) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return response;
    }

    // Read the response data.
    std::string utf8Response;
    char buffer[4096];
    DWORD dwRead = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &dwRead) && dwRead > 0) {
        utf8Response.append(buffer, dwRead);
    }

    // Convert the UTF-8 response (std::string) to a wide string.
    if (!utf8Response.empty()) {
        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Response.c_str(), (int)utf8Response.size(), NULL, 0);
        std::wstring wresponse(sizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8Response.c_str(), (int)utf8Response.size(), &wresponse[0], sizeNeeded);
        response = wresponse;
    }

    // Cleanup handles.
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}

// Retrieves the JSON response using the provided userkey, extracts the filename and file data,
// and writes the file data (binary data stored as a string) to a file with the given filename.
bool RetrieveAndWriteFile(const std::wstring& userkey) {
    // Construct the resource URL by appending the userkey.
    std::wstring resource = ENDPOINT_BASE + userkey;

    // Retrieve the server's response.
    std::wstring wresponse = HttpGetRequest(resource);
    if (wresponse.empty()) {
        std::wcerr << L"Failed to receive a response from the server." << std::endl;
        return false;
    }

    // Convert the wide string response to std::string (UTF-8) for JSON parsing.
    std::string jsonStr(wresponse.begin(), wresponse.end());
    json j;
    try {
        j = json::parse(jsonStr);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }

    // Ensure the JSON response contains both "filename" and "file".
    if (!j.contains("filename") || !j.contains("file")) {
        std::cerr << "JSON response missing required keys." << std::endl;
        return false;
    }

    // Extract the filename and file data.
    std::string filename = j["filename"];
    std::string fileData = j["file"];  // This is binary data stored as a string.

    // Open a file handle for writing in binary mode.
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return false;
    }

    // Write the binary data directly to the file.
    outFile.write(fileData.data(), fileData.size());
    outFile.close();

    std::wcout << L"File \"" << std::wstring(filename.begin(), filename.end())
               << L"\" written successfully." << std::endl;
    return true;
}
