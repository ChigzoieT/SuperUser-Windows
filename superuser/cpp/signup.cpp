#define _WIN32_IE 0x0501
#define IDC_TEXTVIEWW 1001
#define IDC_FRAME     1002
#define IDT_ERROR_TIMER 3000  // Timer ID for error message display

#include "h/signup.h"
#include <windows.h>    // Explicit include for windows.h
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>
#include <string>
#include "h/connector.h"
#include "h/font_utils.h"
#include "h/saveuser.h"

#pragma comment(lib, "comctl32.lib")

// Global control handles.
HWND hSignupWindow      = nullptr;
HWND hUsernameLabel     = nullptr;
HWND hUsername          = nullptr;
HWND hPasswordLabel     = nullptr;
HWND hPassword          = nullptr;
HWND hAccountLabel      = nullptr;   // New: AccountType label
HWND hAccountDropdown   = nullptr;   // New: Dropdown for account type
HWND hSubmit            = nullptr;
HWND Frames             = nullptr;
HWND hTextVieww         = nullptr;

bool signupSuccess      = false;
bool isButtonHovered    = false;

HINSTANCE hInst = NULL;  // Global instance handle

// Load RichEdit DLL globally.
static void LoadRichEditLibrary() {
    static bool loaded = false;
    if (!loaded) {
        LoadLibrary(L"Msftedit.dll"); // Use "Riched20.dll" if targeting RichEdit 2.0
        loaded = true;
    }
}

//
// Helper function to center the text in the RichEdit control.
//
void CenterRichEditText() {
    PARAFORMAT pf = {0};
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_ALIGNMENT;
    pf.wAlignment = PFA_CENTER;
    // Select all text, apply the paragraph format, then unselect.
    SendMessage(hTextVieww, EM_SETSEL, 0, -1);
    SendMessage(hTextVieww, EM_SETPARAFORMAT, SCF_ALL, (LPARAM)&pf);
    SendMessage(hTextVieww, EM_SETSEL, -1, -1);
}

//
// Helper function to show an error message in the RichEdit control.
// It sets the text and background color, centers the text, and starts a timer for 5 seconds.
//
void ShowErrorInRichEdit(HWND hwnd, LPCWSTR errorText, COLORREF color) {
    SetWindowTextW(hTextVieww, errorText);
    SendMessage(hTextVieww, EM_SETBKGNDCOLOR, FALSE, (LPARAM)color);
    CenterRichEditText();
    SetTimer(hwnd, IDT_ERROR_TIMER, 5000, NULL);
}

//
// ButtonSubclassProc: Handles mouse events (hover effect) for the submit button.
//
LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                      UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/)
{
    switch (msg)
    {
        case WM_MOUSEMOVE:
        {
            // Start tracking WM_MOUSELEAVE for the button.
            TRACKMOUSEEVENT tme = {0};
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);

            if (!isButtonHovered)
            {
                isButtonHovered = true;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            SetCursor(LoadCursor(NULL, IDC_HAND));
            break;
        }
        case WM_MOUSELEAVE:
            if (isButtonHovered)
            {
                isButtonHovered = false;
                InvalidateRect(hwnd, NULL, TRUE);
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            break;
        case WM_NCDESTROY:
            // Remove subclass when the button is destroyed.
            RemoveWindowSubclass(hwnd, ButtonSubclassProc, uIdSubclass);
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

//
// SignupProc: Window procedure for the signup window.
//


void CreateControls(HWND hwnd)
{
    // Load the custom font.
    HFONT hFont = LoadCustomFont();
    LoadRichEditLibrary();

    // Fixed dimensions as provided:
    // Label width: 80, Gap: 10, Edit width: 200 (total = 290), window client width = 420.
    const int totalPairWidth = 290;
    const int clientWidth = 420;
    const int offsetX = (clientWidth - totalPairWidth) / 2;

    // Create username row.
    hUsernameLabel = CreateWindowW(L"STATIC", L"Username:",
                                   WS_VISIBLE | WS_CHILD,
                                   offsetX, 30, 80, 25,
                                   hwnd, NULL, NULL, NULL);
    hUsername = CreateWindowW(L"EDIT", L"",
                              WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                              offsetX + 80 + 10, 30, 200, 25,
                              hwnd, NULL, NULL, NULL);

    // Create password row.
    hPasswordLabel = CreateWindowW(L"STATIC", L"Password:",
                                   WS_VISIBLE | WS_CHILD,
                                   offsetX, 70, 80, 25,
                                   hwnd, NULL, NULL, NULL);
    hPassword = CreateWindowW(L"EDIT", L"",
                              WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
                              offsetX + 80 + 10, 70, 200, 25,
                              hwnd, NULL, NULL, NULL);

    // Create AccountType row.
    hAccountLabel = CreateWindowW(L"STATIC", L"Account Type:",
                                  WS_VISIBLE | WS_CHILD,
                                  offsetX, 110, 80, 25,
                                  hwnd, NULL, NULL, NULL);
    hAccountDropdown = CreateWindowW(L"COMBOBOX", L"",
                                     WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
                                     offsetX + 80 + 10, 110, 200, 80,
                                     hwnd, NULL, NULL, NULL);
    // Populate dropdown.
    SendMessage(hAccountDropdown, CB_ADDSTRING, 0, (LPARAM)L"old");
    SendMessage(hAccountDropdown, CB_ADDSTRING, 0, (LPARAM)L"new");

    // Create submit button.
    hSubmit = CreateWindowW(L"BUTTON", L"Submit",
                            WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                            (clientWidth - 120) / 2, 150, 110, 30,
                            hwnd, (HMENU)1, NULL, NULL);

    // Create frame at the bottom.
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    int clientH = rcClient.bottom - rcClient.top;
    int clientW_actual = rcClient.right - rcClient.left;
    const int frameHeight = 30;
    Frames = CreateWindowW(L"STATIC", NULL,
                           WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                           0, clientH - frameHeight, clientW_actual, frameHeight,
                           hwnd, (HMENU)IDC_FRAME, hInst, NULL);
    // Create RichEdit control inside the frame.
    hTextVieww = CreateWindowEx(0, L"RICHEDIT50W", L"Welcome to SuperUser", 
                                 WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL |
                                 ES_WANTRETURN | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY,
                                 0, 0, clientW_actual, frameHeight,
                                 Frames, (HMENU)IDC_TEXTVIEWW, hInst, NULL);
    // Set fonts and colors.
    SendMessage(hUsernameLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hUsername, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hPasswordLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAccountLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hAccountDropdown, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hSubmit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(Frames, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hTextVieww, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hTextVieww, EM_SETBKGNDCOLOR, FALSE, (LPARAM)RGB(255, 140, 0));
    CenterRichEditText();
    CHARFORMAT2 cf = {0};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = RGB(255, 255, 255);
    SendMessage(hTextVieww, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    SetWindowSubclass(hSubmit, ButtonSubclassProc, 0, 0);
}


LRESULT CALLBACK SignupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
      case WM_CREATE:
        {
            CreateControls(hwnd);
            break;
        }

        case WM_CTLCOLORSTATIC:
        {
            // Remove background for static controls.
            HDC hdcStatic = (HDC)wParam;
            HWND hStatic = (HWND)lParam;
            if (hStatic == hUsernameLabel || hStatic == hPasswordLabel || hStatic == hAccountLabel)
            {
                SetBkMode(hdcStatic, TRANSPARENT);
                return (LRESULT)GetStockObject(NULL_BRUSH);
            }
            break;
        }

    case WM_COMMAND:
        {

        if (LOWORD(wParam) == 1) // Submit button clicked.
        {
            wchar_t username[50] = {0};
            wchar_t password[50] = {0};
            wchar_t accountType[50] = {0};
            GetWindowTextW(hUsername, username, 50);
            GetWindowTextW(hPassword, password, 50);
            int accountSel = (int)SendMessage(hAccountDropdown, CB_GETCURSEL, 0, 0);

            // Validate fields individually.
            if (username[0] == L'\0') {
                ShowErrorInRichEdit(hwnd, L"Username cannot be empty!", RGB(255, 0, 0));
                break;
            }

            else if (password[0] == L'\0') {
                ShowErrorInRichEdit(hwnd, L"Password cannot be empty!", RGB(255, 0, 0));
                break;
            }

            else if (accountSel == CB_ERR || accountSel == -1) {
                ShowErrorInRichEdit(hwnd, L"Account type not selected!", RGB(255, 0, 0));
            break;
        }


        // Convert wchar_t to std::string (UTF-8)
        int usernameLen = WideCharToMultiByte(CP_UTF8, 0, username, -1, NULL, 0, NULL, NULL);
        int passwordLen = WideCharToMultiByte(CP_UTF8, 0, password, -1, NULL, 0, NULL, NULL);

        std::string usernameStr(usernameLen, 0);
        std::string passwordStr(passwordLen, 0);

        WideCharToMultiByte(CP_UTF8, 0, username, -1, &usernameStr[0], usernameLen, NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, password, -1, &passwordStr[0], passwordLen, NULL, NULL);

        // Buffer to store selected account type
        SendMessageW(hAccountDropdown, CB_GETLBTEXT, accountSel, (LPARAM)accountType);

        // Convert accountType from wchar_t to UTF-8 std::string
        int stateLen = WideCharToMultiByte(CP_UTF8, 0, accountType, -1, NULL, 0, NULL, NULL);
        std::string state(stateLen, 0);
        WideCharToMultiByte(CP_UTF8, 0, accountType, -1, &state[0], stateLen, NULL, NULL);
        state.pop_back(); // Remove the null terminator

        std::string cmptext = usernameStr; // Same as username

        // Send data to the server
        std::string response = sendToPHP(usernameStr, passwordStr, state, cmptext);
        if (response.find("welcome") != std::string::npos) {
            if (state == "new") {
                saveUserKeyIfWelcome(response);
            }

            signupSuccess = true;
            std::wstring responseW(response.begin(), response.end());
            ShowErrorInRichEdit(hwnd, responseW.c_str(), RGB(0, 128, 0));
            // Force the window to update its contents.
            InvalidateRect(hwnd, NULL, TRUE);
            UpdateWindow(hwnd);
            Sleep(5000);  // Wait so the user can read the message.
            PostMessage(hwnd, WM_CLOSE, 0, 0);

        } else {
             std::wstring responseW(response.begin(), response.end());
            ShowErrorInRichEdit(hwnd, responseW.c_str(), RGB(255, 0, 0));
        }
    }
    break;
}



        case WM_TIMER:
        {
            if (wParam == IDT_ERROR_TIMER)
            {
                // Reset the RichEdit control to its default text and background.
                SetWindowTextW(hTextVieww, L"Welcome to SuperUser");
                SendMessage(hTextVieww, EM_SETBKGNDCOLOR, FALSE, (LPARAM)RGB(255, 140, 0));
                CenterRichEditText();
                KillTimer(hwnd, IDT_ERROR_TIMER);
            }
            break;
        }

        case WM_DRAWITEM:
        {
            if (wParam == 1)
            {
                // Draw the submit button.
                LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
                HDC hdc = dis->hDC;
                RECT rect = dis->rcItem;
                HBRUSH hBrush = CreateSolidBrush(isButtonHovered ? RGB(135, 206, 250) : RGB(255, 255, 255));
                FillRect(hdc, &rect, hBrush);

                // Draw a rounded, light-gray border.
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 15, 15);
                SelectObject(hdc, hOldBrush);
                SelectObject(hdc, hOldPen);

                // Draw centered button text.
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(0, 0, 0));
                DrawTextW(hdc, L"Submit", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                DeleteObject(hBrush);
                DeleteObject(hPen);
            }
            return TRUE;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}


bool ShowSignupWindow(HINSTANCE hInstance)
{
    hInst = hInstance;  // Assign the instance handle globally.
    
    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = SignupProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"SignupWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(101));

    if (!RegisterClassW(&wc))
    {
        MessageBoxW(nullptr, L"Failed to register window class!", L"Error", MB_ICONERROR);
        return false;
    }

    // Window dimensions.
    const int windowWidth = 420;
    const int windowHeight = 250;
    // Get screen dimensions.
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    // Calculate top-left corner to center the window.
    int posX = (screenWidth - windowWidth) / 2;
    int posY = (screenHeight - windowHeight) / 2;

    hSignupWindow = CreateWindowW(L"SignupWindow", L"Sign Up",
                                  WS_OVERLAPPED | WS_SYSMENU,
                                  posX, posY, windowWidth, windowHeight,
                                  nullptr, nullptr, hInstance, nullptr);

    if (!hSignupWindow)
    {
        MessageBoxW(nullptr, L"Failed to create signup window!", L"Error", MB_ICONERROR);
        return false;
    }

    ShowWindow(hSignupWindow, SW_SHOW);
    UpdateWindow(hSignupWindow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return signupSuccess;
}
