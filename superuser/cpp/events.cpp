#include "../h/events.h"
#include "../h/superuser.h"
#include "../h/image_converter.h"
#include "../h/retrievedata.h"
#include "../h/uploaddata.h"
#include "../h/depressorfile.h"
#include "../h/pressorfile.h"
#include "../h/pressortext.h"
#include "../h/audio_converter.h"
#include "../h/video_converter.h"
#include "../h/cpu_info_plugger.h"
#include "../h/image_converter.h"
#include "../h/cpu_info_plugger.h"
#include "../h/saveuser.h"
#include <commdlg.h>
#include <windows.h>
#include <new>
#include "../h/main.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <filesystem>

//selecteditem
//threaddata

int position = 0;

std::vector<std::wstring> filenames;
std::vector<std::wstring> paths;
std::vector<std::wstring> fullpaths;
//void set_thread_count(AVCodecContext* codec_ctx, int thread_count);
//int convert_audio(const char* input_filename, const char* output_filename, int thread_count);
//void convert_video_to_h265(const char* input_file, const char* output_file, int thread_count);
//void decompressFileWithExtension(const char* inputFilePath, const char* outputFilePath, int numThreads);
//void compressFileWithExtension(const char* inputFilePath, const char* outputFilePath, int numThreads);
//void convert_image(const char* input_file, const char* output_file);

using namespace std;

void updateTextView1(const std::string& text) { 
    if (!IsWindow(hTextView1)) { 
        MessageBox(NULL, L"hTextView1 is not a valid window!", L"Error", MB_OK | MB_ICONERROR); 
        return; 
    } 
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0); 
    if (len == 0) return; 
    std::wstring wtext(len, L'\0'); 
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wtext[0], len); 
    PARAFORMAT pf; 
    ZeroMemory(&pf, sizeof(pf)); 
    pf.cbSize = sizeof(PARAFORMAT); 
    pf.dwMask = PFM_ALIGNMENT; 
    pf.wAlignment = PFA_CENTER; 
    SendMessage(hTextView1, EM_SETPARAFORMAT, 0, (LPARAM)&pf); 
    SendMessage(hTextView1, WM_SETTEXT, 0, (LPARAM)wtext.c_str()); 
}

std::string incrementPosition() {
    position += 1;
    std::string result = std::to_string(position);
    updateTextView1(result);
    return super_user + result;
}

std::string decrementPosition() {
    if (position > 0) {
        position -= 1;
    }
    std::string result = std::to_string(position);
    updateTextView1(result);
    return super_user + result;
}

std::string updatefilevalue(HWND hFileValue, const std::vector<std::wstring>& filenames) {
    if (filenames.empty()) {
        SetWindowTextW(hFileValue, L""); // Clear the window text
        return ""; // Return an empty string
    } else if (filenames.size() == 1) {
        SetWindowTextW(hFileValue, filenames[0].c_str()); // Set the window text to the single filename
        return std::string(filenames[0].begin(), filenames[0].end()); // Return the filename as std::string
    } else {
        SetWindowTextW(hFileValue, L"Multiple"); // Set the window text to "Multiple"
        return "Multiple Files not Allowed!"; // Return this message
    }
}


void checker() {
    std::vector<std::thread> threads;

    for (size_t i = 0; i < fullpaths.size(); i++) {
        const auto& currentFullPath = fullpaths[i];

        if (currentFullPath.empty()) {
            validateclickzWithTimer(hwnd, L"No file selected!", RGB(255, 0, 0), 3000);
            return;  // Exit the function immediately
        }

        threads.emplace_back([currentFullPath]() {
            try {
                if (selecteditem == L"File") {
                    compressFileWithExtension(hwnd, currentFullPath, threaddata);
                } else if (selecteditem == L"Image") {
                    convert_image(hwnd, currentFullPath, threaddata);
                } else if (selecteditem == L"Audio") {
                    convert_audio(hwnd, currentFullPath, threaddata);
                } else if (selecteditem == L"Video") {
                    convert_video(hwnd, currentFullPath, threaddata);
                }
            } catch (const std::exception& e) {
                std::wstring errorMessage = L"Error processing full path: " + currentFullPath + L"\n" +
                                            L"Error: " + std::wstring(e.what(), e.what() + strlen(e.what()));
                MessageBox(hwnd, errorMessage.c_str(), L"Error", MB_ICONERROR);
            }
        });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void validateclick(HWND hwnd, const wchar_t* text, COLORREF color) {
    SendMessage(hTextView2, WM_SETTEXT, 0, (LPARAM)L"");
    SendMessage(hTextView2, WM_SETTEXT, 0, (LPARAM)text);
    SendMessage(hTextView2, EM_SETBKGNDCOLOR, 0, (LPARAM)color);
    SetTimer(hwnd, TIMER_ID, 5000, NULL);

    PARAFORMAT pf = { sizeof(PARAFORMAT) };
    pf.dwMask = PFM_ALIGNMENT;
    pf.wAlignment = PFA_CENTER;
    SendMessage(hTextView2, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
}

void OnButtonUpdateClicked(HWND hwnd) {
    int textLength = GetWindowTextLength(hTextView);
    wchar_t* buffer = new wchar_t[textLength + 1];

    GetWindowText(hTextView, buffer, textLength + 1);

    // Store retrieved text
    std::wstring inputText = buffer;
    delete[] buffer;

    // Convert wstring to string before compression
    std::string narrowInput(inputText.begin(), inputText.end());

    // Get compressed data
    std::string compressedText = CompressData(narrowInput);
}


void OnButtonCopyClicked(HWND hwnd) {
    int len = GetWindowTextLengthW(hTextView);
    if (len == 0) return;

    len++; 
    WCHAR* buffer = (WCHAR*)GlobalAlloc(GMEM_FIXED, len * sizeof(WCHAR));
    if (!buffer) return;

    GetWindowTextW(hTextView, buffer, len);

    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(WCHAR));
        if (hClipboardData) {
            WCHAR* pClipboard = (WCHAR*)GlobalLock(hClipboardData);
            if (pClipboard) {
                memcpy(pClipboard, buffer, len * sizeof(WCHAR));
                GlobalUnlock(hClipboardData);
                SetClipboardData(CF_UNICODETEXT, hClipboardData);
            }
        }
        CloseClipboard();
    }

    GlobalFree(buffer);
    validateclick(hwnd, L"Text Copied!\n", RGB(100, 149, 237));
}

void OnButtonCompressClicked(HWND hwnd) {
    SendMessage(hTextView, EM_REPLACESEL, 0, (LPARAM)L"Compress button clicked\n");
}

void OnButtonDecompressClicked(HWND hwnd) {
    SendMessage(hTextView, EM_REPLACESEL, 0, (LPARAM)L"Decompress button clicked\n");
}

std::atomic<int> latestRequestId{0};  // Tracks the latest request
std::mutex retrieveMutex;  // Ensures only one retrieval executes

void RetrieveLatest(HWND hwnd, const std::string& key, int requestId) {
    if (requestId != latestRequestId.load()) {  
        return;  // Ignore outdated requests
    }

    std::lock_guard<std::mutex> lock(retrieveMutex);  // Prevent simultaneous executions
    RetrieveTextAndFilename(hwnd, key);  // Pass HWND correctly

    if (requestId == latestRequestId.load()) {
        // Process the result (if necessary)
    }
}

void OnButtonPrevClicked(HWND hwnd) {
    int requestId = ++latestRequestId;  // Increment request ID (invalidate older calls)
    std::string key = decrementPosition();  

    std::thread(RetrieveLatest, hwnd, key, requestId).detach();
}

void OnButtonNextClicked(HWND hwnd) {
    int requestId = ++latestRequestId;
    std::string key = incrementPosition();  

    std::thread(RetrieveLatest, hwnd, key, requestId).detach();
}


void OnButtonAttachFileClicked(HWND hwnd) {
    OPENFILENAME ofn;
    WCHAR szFile[4096]; // Increase buffer size to handle multiple files

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0'; // Initialize buffer
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]); // Set buffer size
    ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.TXT\0"; // File filter
    ofn.nFilterIndex = 1; // Default filter index
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = L"Select Files";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (GetOpenFileName(&ofn) == TRUE) {
        // Clear the global arrays before adding new files
        filenames.clear();
        paths.clear();
        fullpaths.clear();

        // Extract the directory path (first item in the buffer)
        std::wstring directoryPath = ofn.lpstrFile;
        WCHAR* pFile = ofn.lpstrFile + directoryPath.length() + 1;

        // Parse all selected files
        while (*pFile != '\0') {
            std::wstring filename = pFile; // Extract filename
            std::wstring fullpath = directoryPath + L"\\" + filename; // Construct full path

            // Add to the global arrays
            filenames.push_back(filename);
            paths.push_back(directoryPath);
            fullpaths.push_back(fullpath);

            pFile += wcslen(pFile) + 1; // Move to the next file
        }

        // If no files were added (single file selected), add the directory path itself
        if (filenames.empty()) {
            std::filesystem::path fsPath(directoryPath);
            filenames.push_back(fsPath.filename().wstring()); // Extract filename
            paths.push_back(fsPath.parent_path().wstring());  // Extract directory path
            fullpaths.push_back(directoryPath);               // Store full path
        }

        // Call updatefilevalue to update the hFileValue static control
        updatefilevalue(hFileValue, filenames);

        // Display selected files in the text view
        SendMessage(hTextView, EM_REPLACESEL, 0, (LPARAM)L"Selected Files:\r\n");
        for (size_t i = 0; i < filenames.size(); i++) {
            std::wstring message = L"File: " + filenames[i] + L"\r\n" +
                                   L"Path: " + paths[i] + L"\r\n" +
                                   L"Full Path: " + fullpaths[i] + L"\r\n\r\n";
            SendMessage(hTextView, EM_REPLACESEL, 0, (LPARAM)message.c_str());
        }
    }
}
void UpdateListBoxWithTextViewContent(HWND hwnd, HWND hTextView, HWND hListBox) {
    int textLength = SendMessage(hTextView, WM_GETTEXTLENGTH, 0, 0);
    if (textLength == 0) {
        MessageBox(hwnd, L"The text view is empty!", L"Empty Text View", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    int lineCount = SendMessage(hTextView, EM_GETLINECOUNT, 0, 0);

    wchar_t message[256];
    wsprintf(message, L"Number of lines: %d", lineCount);
    MessageBox(hwnd, message, L"Text Change Debug", MB_OK);

    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

    wchar_t lineCountStr[32];
    wsprintf(lineCountStr, L"Lines: %d", lineCount);
    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)lineCountStr);

    for (int i = 0; i < lineCount; ++i) {
        int lineLength = SendMessage(hTextView, EM_LINELENGTH, i, 0);
        wchar_t* lineText = new(std::nothrow) wchar_t[lineLength + 1];
        if (lineText != nullptr) {
            SendMessage(hTextView, EM_GETLINE, i, (LPARAM)lineText);
            lineText[lineLength] = '\0';

            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)lineText);
            delete[] lineText;
        }
    }
}

void OnButtonDeleteClicked(HWND hwnd) {
    SendMessage(hTextView, EM_REPLACESEL, 0, (LPARAM)L"Delete button clicked\n");
}
