#ifndef DELETEDATA_H
#define DELETEDATA_H

#include <windows.h> // For HWND and RGB
#include <string>    // For std::string and std::wstring

// Forward declaration of the DeleteDataByUserkey function
void DeleteDataByUserkey(HWND hwnd, const std::wstring &userkey);

// Helper function: Converts std::wstring to std::string (assumes ASCII)
static std::string ConvertWStringToString(const std::wstring &wstr);

// Callback function to capture the response from the server
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);


#endif // DELETEDATA_H