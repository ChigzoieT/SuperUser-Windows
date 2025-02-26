#ifndef RETRIEVEDATA_H
#define RETRIEVEDATA_H

#include <windows.h>
#include <string>

// Global handles for UI elements
extern HWND hTextView;
extern HWND hFileValue;

// Function declarations
void HandleError(HWND hwnd);
void setElement(HWND hwnd, const std::string& text, const std::string& filename);
void RetrieveTextAndFilename(HWND hwnd, const std::string& userkey);
std::wstring stringToWstring(const std::string& str); // Function declaration

#endif // RETRIEVEDATA_H
