#define _WIN32_IE 0x0501

#include "h/components.h"
#include "h/saveuser.h"
#include "h/retrievedata.h"
#include "h/allocator.h"
#include <richedit.h>
#include "h/cpu_info_plugger.h"
#include "h/font_utils.h"
#include "h/superuser.h"
#include "h/button_subclass.h"
#include <windows.h>
#include "h/prevnext.h"
#include <Commctrl.h>

// Global variables for the buttons, listbox/edit controls, and frame
WNDPROC g_originalButtonProcUpdate = nullptr;
WNDPROC g_originalButtonProcCopy = nullptr;
WNDPROC g_originalButtonProcCompress = nullptr;
WNDPROC g_originalButtonProcDecompress = nullptr;
WNDPROC g_originalButtonProcAttach = nullptr;
WNDPROC g_originalButtonProcNext = nullptr;
WNDPROC g_originalButtonProcPrev  = nullptr;
WNDPROC g_originalButtonProcDelete = nullptr;


// Load RichEdit DLL globally
void LoadRichEditLibrary() {
    static bool loaded = false;
    if (!loaded) {
        LoadLibrary(L"Msftedit.dll"); // Use "Riched20.dll" if targeting RichEdit 2.0
        loaded = true;
    }
}


void InitializeSuperUserComponents(HWND hwnd, HWND &hTextView, HWND &hButtonUpdate, HWND &hButtonCopy, 
    HWND &hButtonCompress, HWND &hButtonDecompress, HWND &hButtonAttach, HWND &hButtonPrev, HWND &hButtonNext, HWND &hComboBox, HWND &hListBox, 
    HWND &hFrame, HWND &hCores, HWND &hCoresValue, HWND &hSpeed, HWND &hSpeedValue, HWND &hThread, HWND &hThreadValue, HWND &hTextView1, HWND &hFrame1, HWND &hTextView2, HWND &hButtonDelete, HWND &hFile, HWND &hFileValue)
{
    // Load RichEdit before creating any RichEdit controls
    LoadRichEditLibrary();


    HINSTANCE hInst = GetModuleHandle(NULL);
    HFONT hFont = LoadCustomFont();
    HFONT hFont1 = LoadRootFont();

    CHARFORMAT2 cf = {0};

    hFrame = CreateWindowW(
        L"STATIC", 
        NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        10, 10, 560, 350, 
        hwnd, 
        (HMENU)IDC_FRAME, 
        hInst, 
        NULL);
    SendMessage(hFrame, WM_SETFONT, WPARAM(hFont), TRUE);

   // Create the RichEdit control

    hTextView = CreateWindowEx(
        0,
        L"RICHEDIT50W",  // Correct class name for "Msftedit.dll"
        L"Awaiting text from you...", 
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | ES_AUTOHSCROLL | WS_HSCROLL,
        25, 5, 315, 340, 
        hFrame, 
        (HMENU)IDC_TEXTVIEW, 
        hInst, 
        NULL
        );

        // Hide the horizontal scrollbar but keep horizontal scrolling
    ShowScrollBar(hTextView, SB_HORZ, FALSE);
    SendMessage(hTextView, EM_SHOWSCROLLBAR, SB_HORZ, FALSE);

    // Apply the loaded font
    SendMessage(hTextView, WM_SETFONT, WPARAM(hFont1), TRUE);

    // Set background color to darkish grey-blue
    SendMessage(hTextView, EM_SETBKGNDCOLOR, FALSE, (LPARAM)RGB(44, 62, 80));

    // Set text color using CHARFORMAT2 without changing font
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;  // Only modify the text color
    cf.crTextColor = RGB(255, 255, 255);  // White text

    SendMessage(hTextView, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

    // Create the Prev button
    hButtonPrev = CreateWindowW(
        L"BUTTON", 
        L"Prev", 
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        50, 100, 100, 30, 
        hwnd, 
        (HMENU)IDC_BUTTON_PREV, 
        hInst, 
        NULL);
    SendMessage(hButtonPrev, WM_SETFONT, WPARAM(hFont), TRUE);
    g_originalButtonProcPrev = (WNDPROC)GetWindowLongPtr(hButtonPrev, GWLP_WNDPROC);
    SetWindowLongPtr(hButtonPrev, GWLP_WNDPROC, (LONG_PTR)PrevButtonProc);


    hTextView1 = CreateWindowW(
        L"RICHEDIT50W", 
        L"1", 
        WS_VISIBLE | WS_CHILD | ES_CENTER | ES_MULTILINE | ES_AUTOVSCROLL, 
        140, 40, 50, 20, 
        hwnd, 
        (HMENU)IDC_TEXTVIEW1, 
        hInst, 
        NULL
        );

    SendMessage(hTextView1, WM_SETFONT, WPARAM(hFont), TRUE);

    // Ensure text is centralized
    PARAFORMAT pf;
    ZeroMemory(&pf, sizeof(pf));
    pf.cbSize = sizeof(PARAFORMAT);
    pf.dwMask = PFM_ALIGNMENT;
    pf.wAlignment = PFA_CENTER;
    SendMessage(hTextView1, EM_SETPARAFORMAT, 0, (LPARAM)&pf);

    // Set text color to black
    //CHARFORMAT cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = RGB(0, 0, 0); // Black color
    SendMessage(hTextView1, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

    // Create the Next button
    hButtonNext = CreateWindowW(
        L"BUTTON", 
        L"Next", 
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        150, 100, 100, 30, 
        hwnd, 
        (HMENU)IDC_BUTTON_NEXT, 
        hInst, 
        NULL);
    SendMessage(hButtonNext, WM_SETFONT, WPARAM(hFont), TRUE);
    g_originalButtonProcNext = (WNDPROC)GetWindowLongPtr(hButtonNext, GWLP_WNDPROC);
    SetWindowLongPtr(hButtonNext, GWLP_WNDPROC, (LONG_PTR)NextButtonProc);

    // Create the Update button
    hButtonUpdate = CreateWindowW(
        L"BUTTON", 
        L"Update", 
        WS_CHILD | WS_VISIBLE,
        10, 210, 100, 30, 
        hwnd, 
        (HMENU)IDC_BUTTON_UPDATE, 
        hInst, 
        NULL);
    SendMessage(hButtonUpdate, WM_SETFONT, WPARAM(hFont), TRUE);
    g_originalButtonProcUpdate = (WNDPROC)GetWindowLongPtr(hButtonUpdate, GWLP_WNDPROC);
    SetWindowLongPtr(hButtonUpdate, GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);

    // Create the Update button
    hButtonDelete = CreateWindowW(
        L"BUTTON", 
        L"Update", 
        WS_CHILD | WS_VISIBLE,
        10, 210, 100, 30, 
        hwnd, 
        (HMENU)IDC_BUTTON_DELETE, 
        hInst, 
        NULL);
    SendMessage(hButtonDelete, WM_SETFONT, WPARAM(hFont), TRUE);
    g_originalButtonProcDelete = (WNDPROC)GetWindowLongPtr(hButtonDelete, GWLP_WNDPROC);
    SetWindowLongPtr(hButtonDelete, GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);


   hButtonCopy = CreateWindowW(
    L"BUTTON", 
    NULL,                    // No text when using an icon-only button.
    WS_CHILD | WS_VISIBLE | BS_ICON,
    120, 210, 100, 30, 
    hwnd, 
    (HMENU)IDC_BUTTON_COPY, 
    hInst, 
    NULL);

// Set the font if needed.
SendMessage(hButtonCopy, WM_SETFONT, WPARAM(hFont), TRUE);

// (Optional) Subclass the button if you need custom behavior.
g_originalButtonProcCopy = (WNDPROC)GetWindowLongPtr(hButtonCopy, GWLP_WNDPROC);
SetWindowLongPtr(hButtonCopy, GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);

// Load the icon from file.
HICON hIcon = (HICON)LoadImage(
    NULL, 
    L"assets\\copy.ico",  // Path to your icon file.
    IMAGE_ICON,
    32, 32,               // Desired width and height.
    LR_LOADFROMFILE | LR_DEFAULTCOLOR
);

// Attach the icon to the button.
SendMessage(hButtonCopy, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);


    // Create the Compress button
    hButtonCompress = CreateWindowW(
        L"BUTTON", 
        L"Compress", 
        WS_CHILD | WS_VISIBLE,
        230, 210, 100, 30, 
        hwnd, 
        (HMENU)IDC_BUTTON_COMPRESS, 
        hInst, 
        NULL);
    SendMessage(hButtonCompress, WM_SETFONT, WPARAM(hFont), TRUE);
    g_originalButtonProcCompress = (WNDPROC)GetWindowLongPtr(hButtonCompress, GWLP_WNDPROC);
    SetWindowLongPtr(hButtonCompress, GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);

    // Create the Decompress button
    hButtonDecompress = CreateWindowW(
        L"BUTTON", 
        L"Decompress", 
        WS_CHILD | WS_VISIBLE,
        10, 250, 100, 30, 
        hwnd, 
        (HMENU)IDC_BUTTON_DECOMPRESS, 
        hInst, 
        NULL);
    SendMessage(hButtonDecompress, WM_SETFONT, WPARAM(hFont), TRUE);
    g_originalButtonProcDecompress = (WNDPROC)GetWindowLongPtr(hButtonDecompress, GWLP_WNDPROC);
    SetWindowLongPtr(hButtonDecompress, GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);

    // Create the Attach File button
    hButtonAttach = CreateWindowW(
        L"BUTTON", 
        L" ", 
        WS_CHILD | WS_VISIBLE,
        230, 250, 100, 30, 
        hwnd, 
        (HMENU)IDC_BUTTON_ATTACH, 
        hInst, 
        NULL);
    SendMessage(hButtonAttach, WM_SETFONT, WPARAM(hFont), TRUE);
    g_originalButtonProcAttach = (WNDPROC)GetWindowLongPtr(hButtonAttach, GWLP_WNDPROC);
    SetWindowLongPtr(hButtonAttach, GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);

    // Create the ComboBox (without owner drawing)
    hComboBox = CreateWindowW(
        WC_COMBOBOX, 
        NULL, 
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        120, 250, 100, 200, 
        hwnd, 
        (HMENU)IDC_COMBO_BOX, 
        hInst, 
        NULL);
    SendMessage(hComboBox, WM_SETFONT, WPARAM(hFont), TRUE);
    //SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Text");
    SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"File");
    SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Image");
    SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Audio");
    SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"Video");
    SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
    SendMessage(hComboBox, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)100);

    // Set focus to the text view
    SetFocus(hTextView);

    // Create labels for Processor Info
    hCores = CreateWindowW(
        L"STATIC", 
        L"Processor Cores:", 
        WS_VISIBLE | WS_CHILD, 
        10, 10, 120, 20, 
        hwnd, 
        (HMENU)IDC_CORES_LABEL, 
        hInst, 
        NULL);
    SendMessage(hCores, WM_SETFONT, WPARAM(hFont), TRUE);

    hCoresValue = CreateWindowW(
        L"STATIC", 
        L"", 
        WS_VISIBLE | WS_CHILD, 
        140, 10, 80, 20, 
        hwnd, 
        (HMENU)IDC_CORES_VALUE, 
        hInst, 
        NULL);
    SendMessage(hCoresValue, WM_SETFONT, WPARAM(hFont), TRUE);

    hSpeed = CreateWindowW(
        L"STATIC", 
        L"Processor Speed:", 
        WS_VISIBLE | WS_CHILD, 
        10, 40, 120, 30, 
        hwnd, 
        (HMENU)IDC_SPEED_LABEL, 
        hInst, 
        NULL);
    SendMessage(hSpeed, WM_SETFONT, WPARAM(hFont), TRUE);

    hSpeedValue = CreateWindowW(
        L"STATIC", 
        L"", 
        WS_VISIBLE | WS_CHILD, 
        140, 40, 80, 30, 
        hwnd, 
        (HMENU)IDC_SPEED_VALUE, 
        hInst, 
        NULL);
    SendMessage(hSpeedValue, WM_SETFONT, WPARAM(hFont), TRUE);

    hThread = CreateWindowW(
        L"STATIC", 
        L"Allocated Threads for Compression:", 
        WS_VISIBLE | WS_CHILD, 
        10, 40, 120, 30, 
        hwnd, 
        (HMENU)IDC_THREAD_LABEL, 
        hInst, 
        NULL);
    SendMessage(hThread, WM_SETFONT, WPARAM(hFont), TRUE);

    hThreadValue = CreateWindowW(
        L"STATIC", 
        L"", 
        WS_VISIBLE | WS_CHILD, 
        140, 40, 80, 30, 
        hwnd, 
        (HMENU)IDC_THREAD_VALUE, 
        hInst, 
        NULL);
    SendMessage(hThreadValue, WM_SETFONT, WPARAM(hFont), TRUE);


    hFile = CreateWindowW(
        L"STATIC", 
        L"File Attached:", 
        WS_VISIBLE | WS_CHILD, 
        10, 40, 120, 30, 
        hwnd, 
        (HMENU)IDC_THREAD_LABEL, 
        hInst, 
        NULL);
    SendMessage(hFile, WM_SETFONT, WPARAM(hFont), TRUE);

    hFileValue = CreateWindowW(
        L"STATIC", 
        L"sdghdsghfdsgfdsgfdgghfdghfdsghfsgfg", 
        WS_VISIBLE | WS_CHILD, 
        140, 40, 80, 30, 
        hwnd, 
        (HMENU)IDC_THREAD_VALUE, 
        hInst, 
        NULL);
    SendMessage(hFileValue, WM_SETFONT, WPARAM(hFont), TRUE);


    hFrame1 = CreateWindowW(
        L"STATIC", 
        NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        10, 10, 560, 350, 
        hwnd, 
        (HMENU)IDC_FRAME, 
        hInst, 
        NULL);
    SendMessage(hFrame1, WM_SETFONT, WPARAM(hFont), TRUE);

    std::wstring displayText = stringToWstring("Hello " + super_user);

    hTextView2 = CreateWindowEx(0, L"RICHEDIT50W",
        displayText.c_str(),
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | 
        ES_WANTRETURN | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY,  
        140, 40, 300, 30, // Adjust size as needed
        hFrame1, 
        (HMENU)IDC_TEXTVIEW2, 
        hInst, 
        NULL
        );


    SendMessage(hTextView2, WM_SETFONT, WPARAM(hFont), TRUE);

    SendMessage(hTextView2, EM_SETBKGNDCOLOR, FALSE, (LPARAM)RGB(255, 140, 0));

    //PARAFORMAT pf = {0};
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_ALIGNMENT;  
    pf.wAlignment = PFA_CENTER;
    SendMessage(hTextView2, EM_SETPARAFORMAT, 0, (LPARAM)&pf);

    //CHARFORMAT2 cf = {0};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;  
    cf.crTextColor = RGB(255, 255, 255);
    SendMessage(hTextView2, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

    // Redraw the window to ensure proper display
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
    UpdateCPUInfo(hwnd, hCoresValue, hSpeedValue, hThreadValue);
}
