// superuser.cpp
#include "../h/superuser.h"
#include "../h/events.h"
#include "../h/resource.h"
#include "../h/components.h"
#include "../h/saveuser.h"
#include "../h/cpu_info.h"
#include <commctrl.h>
#include <windows.h>
#include <commdlg.h>
#include <richedit.h>
#include <string>
#include <sstream>

// Global variables for your controls
HWND hButtonUpdate, hButtonCopy, hButtonCompress, hComboBox;
HWND hTextView, hButtonDecompress, hButtonAttach, hFrame, hListBox, hTextView1, hFrame1, hTextView2, hButtonDelete;
HWND hCores, hCoresValue, hSpeed, hSpeedValue, hThread, hThreadValue, hButtonPrev, hButtonNext, hFile, hFileValue;

std::wstring selecteditem;

// Helper function to set and center text in a rich edit control
void SetAndCenterText(HWND hTextView, const std::string& super_user) {
    SendMessage(hTextView, WM_SETTEXT, 0, (LPARAM)L"");
    std::wstring newText = L"Hello " + std::wstring(super_user.begin(), super_user.end());
    SendMessage(hTextView, EM_REPLACESEL, 0, (LPARAM)newText.c_str());
    PARAFORMAT pf = { sizeof(PARAFORMAT) };
    pf.dwMask = PFM_ALIGNMENT;
    pf.wAlignment = PFA_CENTER;
    SendMessage(hTextView, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
}

// The window procedure for the SuperUser window
LRESULT CALLBACK SuperUserWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // Initialize your components here.
            InitializeSuperUserComponents(hwnd, hTextView, hButtonUpdate, hButtonCopy,
                                          hButtonCompress, hButtonDecompress, hButtonAttach, 
                                          hButtonPrev, hButtonNext, hComboBox, hListBox, hFrame, 
                                          hCores, hCoresValue, hSpeed, hSpeedValue, hThread, 
                                          hThreadValue, hTextView1, hFrame1, hTextView2, hButtonDelete, hFile, hFileValue);

            SetFocus(hTextView);
            SendMessage(hComboBox, CB_SETCURSEL, 0, 0); // Select the first item
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_TEXTVIEW:
                    if (HIWORD(wParam) == EN_SETFOCUS) {
                        MessageBox(hwnd, L"The TextView is active!", L"TextView Active", MB_OK | MB_ICONINFORMATION);
                    }
                    if (HIWORD(wParam) == EN_CHANGE) {
                        UpdateListBoxWithTextViewContent(hwnd, hTextView, hListBox);
                    }
                    break;
                
                case IDC_BUTTON_UPDATE:
                    OnButtonUpdateClicked(hwnd);
                    break;
                
                case IDC_BUTTON_COPY:
                    OnButtonCopyClicked(hwnd);
                    break;
                
                case IDC_BUTTON_COMPRESS:
                    OnButtonCompressClicked(hwnd);
                    break;
                
                case IDC_BUTTON_DECOMPRESS:
                    OnButtonDecompressClicked(hwnd);
                    break;
                
                case IDC_BUTTON_ATTACH:
                    OnButtonAttachFileClicked(hwnd);
                    break;
                
                case IDC_BUTTON_PREV:
                    OnButtonPrevClicked(hwnd);
                    break;
                
                case IDC_BUTTON_NEXT:
                     OnButtonNextClicked(hwnd);
                     break;

                case IDC_BUTTON_DELETE:
                    OnButtonDeleteClicked(hwnd);
                    break;
                
                case IDC_COMBO_BOX:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        int selectedIndex = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
                        wchar_t buffer[256];
                        SendMessage(hComboBox, CB_GETLBTEXT, selectedIndex, (LPARAM)buffer);
                        selecteditem = buffer;
                    }

                    break;
            }
            break;

        case WM_SIZE: {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            ResizeSuperUserComponents(hwnd, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
            break;
        }

        case WM_HSCROLL: {
            int scrollPos = GetScrollPos(hTextView, SB_HORZ);
            int cmd = LOWORD(wParam);
            if (cmd == SB_LINELEFT) scrollPos -= 10;
            if (cmd == SB_LINERIGHT) scrollPos += 10;
            SetScrollPos(hTextView, SB_HORZ, scrollPos, TRUE);
            SendMessage(hTextView, EM_LINESCROLL, scrollPos, 0);
            break;
        }

      case WM_CTLCOLORSTATIC: {
    HWND hControl = (HWND)lParam;
    HBRUSH hBrush = NULL;
    if (hControl == hTextView1) {
        //hBrush = CreateSolidBrush(RGB(255, 255, 0));  // White background for hFileValue
        //SetTextColor((HDC)wParam, RGB(100, 149, 237));         // Fine blue text color for hFileValue
    } else if (hControl == hCores || hControl == hCoresValue || hControl == hSpeed ||
               hControl == hSpeedValue || hControl == hThread || hControl == hThreadValue ||
               hControl == hTextView1 || hFile || hFileValue) {
        hBrush = CreateSolidBrush(RGB(255, 255, 255));
    } else if (hControl == hFrame1) {
        hBrush = CreateSolidBrush(RGB(255, 140, 0));
    }
    if (hBrush) {
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)hBrush;
    }
    break;
}




    case WM_TIMER:
    if (wParam == TIMER_ID) {
        SendMessage(hTextView2, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(255, 140, 0));
        SetAndCenterText(hTextView2, super_user);
        KillTimer(hwnd, TIMER_ID);
    }
    break;

    break;

        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

ATOM RegisterSuperUserClass(HINSTANCE hInstance) {
    WNDCLASSEX wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = SuperUserWndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"SuperUserWindowClass";
    wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    
    return RegisterClassEx(&wc);
}

HWND CreateSuperUserWindow(HINSTANCE hInstance, int nCmdShow) {
    RegisterSuperUserClass(hInstance);
    
    HWND hWnd = CreateWindowEx(
        0,
        L"SuperUserWindowClass",       // Window class name
        L"Super User",                 // Window title
        WS_OVERLAPPEDWINDOW,           // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // Position and size
        NULL,                          // Parent window
        NULL,                          // Menu
        hInstance,                     // Instance handle
        NULL                           // Additional application data
    );
    
    if (hWnd) {
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
    }
    
    return hWnd;
}



void ResizeSuperUserComponents(HWND hwnd, int width, int height) {
    // --- Frame 1 Setup ---
    int frameWidth = width * 0.99;
    int frameHeight = height * 0.6;
    int frameX = (width - frameWidth) / 2;
    int frameY = 0;  // same as height * 0
    MoveWindow(hFrame, frameX, frameY, frameWidth, frameHeight, TRUE);

    RECT frameRect;
    GetClientRect(hFrame, &frameRect);
    int frameClientWidth = frameRect.right;
    int frameClientHeight = frameRect.bottom;

    int padding = 2;
    int tinySpace = 2;
    int usableWidth = frameClientWidth - (2 * padding);
    int usableHeight = frameClientHeight - (2 * padding);

    // --- ListBox Setup ---
    int listBoxWidth = usableWidth * 0.05;
    int listBoxHeight = usableHeight;
    int listBoxX = padding;
    int listBoxY = padding;
    MoveWindow(hListBox, listBoxX, listBoxY, listBoxWidth, listBoxHeight, TRUE);

    // --- TextView Setup ---
    int textViewWidth = usableWidth - listBoxWidth - tinySpace;
    int textViewHeight = usableHeight;
    int textViewX = listBoxX + listBoxWidth + tinySpace;
    int textViewY = padding;
    MoveWindow(hTextView, textViewX - listBoxWidth, textViewY, textViewWidth + listBoxWidth, textViewHeight, TRUE);

    // --- Button Setup ---
    // Navigation buttons (Prev, TextView1, Next)
    int buttonWidth1 = width * 0.1;
    int buttonWidth2 = width * 0.1;
    int buttonHeight = height * 0.05;
    int spacing = width * 0.03;
    int buttonRowY = frameY + frameHeight + spacing;
    int buttonRowZ = frameY + frameHeight + (width * 0.045);

    int totalNavWidth = buttonWidth1 + buttonWidth2 + buttonWidth1 + 2 * spacing;
    int navStartX = (width - totalNavWidth) / 2;

    MoveWindow(hButtonPrev, navStartX, buttonRowY, buttonWidth1, buttonHeight, TRUE);
    MoveWindow(hTextView1, navStartX + buttonWidth1 + spacing, buttonRowZ, buttonWidth2, buttonHeight * 0.6, TRUE);
    MoveWindow(hButtonNext, navStartX + buttonWidth1 + spacing + buttonWidth2 + spacing, buttonRowY, buttonWidth1, buttonHeight, TRUE);

    // --- Icon Buttons Setup (Copy, Upload/Update, Delete, Attach) ---
    int iconButtonWidth = width * 0.1;   // Adjust as needed
    int iconButtonHeight = buttonHeight; // same height as navigation buttons
    int iconSpacing = spacing;           // horizontal gap between icon buttons
    int totalIconWidth = 4 * iconButtonWidth + 3 * iconSpacing;
    int iconStartX = (width - totalIconWidth) / 2;
    int iconRowY = buttonRowY + buttonHeight + spacing;

    MoveWindow(hButtonCopy, iconStartX, iconRowY, iconButtonWidth, iconButtonHeight, TRUE);
    MoveWindow(hButtonUpdate, iconStartX + (iconButtonWidth + iconSpacing), iconRowY, iconButtonWidth, iconButtonHeight, TRUE);
    MoveWindow(hButtonDelete, iconStartX + 2 * (iconButtonWidth + iconSpacing), iconRowY, iconButtonWidth, iconButtonHeight, TRUE);
    MoveWindow(hButtonAttach, iconStartX + 3 * (iconButtonWidth + iconSpacing), iconRowY, iconButtonWidth, iconButtonHeight, TRUE);

    // --- Text-based Buttons Setup (Compress, Decompress, ComboBox) ---
    int buttonWidth = width * 0.23;  // width for these text buttons
    int totalTextWidth = 3 * buttonWidth + 2 * spacing;
    int textStartX = (width - totalTextWidth) / 2;
    int textRowY = iconRowY + iconButtonHeight + spacing;

    MoveWindow(hButtonCompress, textStartX, textRowY, buttonWidth, buttonHeight, TRUE);
    MoveWindow(hButtonDecompress, textStartX + buttonWidth + spacing, textRowY, buttonWidth, buttonHeight, TRUE);
    MoveWindow(hComboBox, textStartX + 2 * (buttonWidth + spacing), textRowY, buttonWidth, buttonHeight * 6, TRUE);

    // --- CPU Info Setup ---
    int labelWidth = buttonWidth * 1.15;
    int labelWidthx = buttonWidth * 1;
    int labelnewwidth = buttonWidth * 2.4;
    int labelnewwidth1 = buttonWidth * 3.0;
    int valueWidth1 = buttonWidth * 0.3;
    int valueWidth = buttonWidth * 0.9;
    int reducedSpacing2 = 1;
    int cpuInfoX = width * 0.04;
    int cpuInfoY = textRowY + buttonHeight + spacing;

    // Cores
    MoveWindow(hCores, cpuInfoX, cpuInfoY, labelWidth, buttonHeight, TRUE);
    MoveWindow(hCoresValue, cpuInfoX + labelWidth + reducedSpacing2, cpuInfoY, valueWidth1, buttonHeight, TRUE);

    // Speed
    int nextLineY = cpuInfoY + (buttonHeight / 1.5);
    MoveWindow(hSpeed, cpuInfoX, nextLineY, labelWidth, buttonHeight / 2, TRUE);
    MoveWindow(hSpeedValue, cpuInfoX + labelWidth + reducedSpacing2, nextLineY, valueWidth, buttonHeight / 2, TRUE);

    // Allocated Number of Threads
    int threadLineY = nextLineY + (buttonHeight / 1.5);
    MoveWindow(hThread, cpuInfoX, threadLineY, labelnewwidth, buttonHeight / 2, TRUE);
    MoveWindow(hThreadValue, cpuInfoX + labelnewwidth + reducedSpacing2, threadLineY, valueWidth1, buttonHeight / 2, TRUE);

    // Allocated Number of Files (placed below threads)
    int fileLineY = threadLineY + (buttonHeight / 4) + spacing;
    MoveWindow(hFile, cpuInfoX, fileLineY, labelWidthx, buttonHeight / 2, TRUE);
    // Bring hFileValue closer to hFile by positioning it relative to labelWidthx instead of labelWidth.
    MoveWindow(hFileValue, cpuInfoX + labelWidthx + reducedSpacing2, fileLineY, labelnewwidth1, buttonHeight / 2, TRUE);

    // --- Frame 2 and hTextView2 Setup ---
    int frame2Height = height * 0.05;
    int frame2Y = height - frame2Height;
    MoveWindow(hFrame1, 0, frame2Y, width, frame2Height, TRUE);

    int hTextView2Width = width;  // Full width of the window
    int hTextView2Height = (int)(frame2Height * 0.8);
    int hTextView2X = 0;
    int hTextView2Y = (frame2Height - hTextView2Height) / 2;
    MoveWindow(hTextView2, hTextView2X, hTextView2Y, hTextView2Width, frame2Height + 2, TRUE);
}
