#ifndef SUPERUSER_H
#define SUPERUSER_H

#include "winsys.h"
#include "cpu_info.h"
#include <vector> 
#include <windows.h>
#include <string>
#include <filesystem>

// Margin for gap on left and right sides
#define MARGIN 10

// Callback function declarations
LRESULT CALLBACK SuperUserWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Functions for handling button events
void OnButtonUpdateClicked(HWND hwnd);
void OnButtonExpandClicked(HWND hwnd);
void OnButtonClearAllClicked(HWND hwnd);
void OnButtonPrevClicked(HWND hwnd);
void OnButtonNextClicked(HWND hwnd);
void OnButtonAttachFileClicked(HWND hwnd);
void OnButtonDeleteClicked(HWND hwnd);

extern HWND hButtonUpdate, hButtonCopy, hButtonCompress, hButtonDecompress, hButtonAttach;
extern HWND hTextView, hComboBox, hListBox, hFrame, hTextView1, hFrame1, hTextView2, hButtonDelete;
extern HWND hCores, hCoresValue, hSpeed, hSpeedValue, hThread, hThreadValue,hButtonPrev, hButtonNext, hFile, hFileValue;

extern std::vector<std::wstring> filenames;
extern std::vector<std::wstring> paths;
extern std::vector<std::wstring> fullpaths;
extern std::wstring selecteditem;

// Utility functions
//void InitializeSuperUserComponents(HWND hwnd);
void UpdateCPUInfo(HWND hwnd);

// Control IDs
#define IDC_BUTTON_UPDATE 101
#define IDC_BUTTON_COPY 102
#define IDC_BUTTON_COMPRESS 103
#define IDC_TEXTVIEW 106
#define IDC_BUTTON_DECOMPRESS 107
#define IDC_BUTTON_ATTACH 109
#define IDC_COMBO_BOX 110 // Add the missing control ID here
#define IDC_FRAME 111
#define IDC_LISTBOX 112
#define IDC_CORES_LABEL 113
#define IDC_CORES_VALUE 114
#define IDC_SPEED_LABEL 115
#define IDC_SPEED_VALUE 116
#define IDC_THREAD_LABEL 117
#define IDC_THREAD_VALUE 118
#define IDC_BUTTON_PREV 119
#define IDC_BUTTON_NEXT 120
#define IDC_TEXTVIEW1 121
#define IDC_FRAME1 122
#define IDC_TEXTVIEW2 123
#define TIMER_ID 124
#define IDC_BUTTON_DELETE  125

// Function for resizing components based on the window size
void ResizeSuperUserComponents(HWND hwnd, int width, int height);

void SetAndCenterText(HWND hTextView, const std::string& super_user);
HWND CreateSuperUserWindow(HINSTANCE hInstance, int nCmdShow);

#endif // SUPERUSER_H
