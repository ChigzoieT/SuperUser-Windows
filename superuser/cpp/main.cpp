#include "h/lessbusycore.h"    // Our module for setting affinity
#include "h/superuser.h"
#include "h/saveuser.h"
#include "h/events.h"
#include "h/fireonboot.h"
#include "h/components.h"
#include <windows.h>
#include <string>
#include <commctrl.h>
#include <tchar.h>
#include "h/resource.h"
#include <richedit.h>
#include "h/main.h"
#include "h/signup.h"
#include <Shlwapi.h>           // For PathRemoveFileSpec
#pragma comment(lib, "Shlwapi.lib")  // Link against Shlwapi.lib

// Global window handle (and any other global controls you use)
HWND hwnd;

// Window procedure for your main window.
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Set the current directory to the directory of the executable.
    TCHAR szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, MAX_PATH)) {
        PathRemoveFileSpec(szPath);
        SetCurrentDirectory(szPath);
    }

    // Ensure the app runs at startup.
    int selectedCore = SetAffinityToLeastBusyCore();
    AddToStartup();

    // Check if a superuser exists before creating any window.
    std::string status = loadSuperUser(); 
    if (status != "exists") {
        // Run the signup process if the superuser does not exist.
        if (!ShowSignupWindow(hInstance)) {
            return 0; // Exit if signup fails.
        }
    }

    // Initialize common controls.
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    // Create the SuperUser window using the function defined in superuser.cpp.
    HWND hwnd = CreateSuperUserWindow(hInstance, nCmdShow);
    if (!hwnd) {
        MessageBox(nullptr, _T("Failed to create the SuperUser window!"), _T("Error"), MB_ICONERROR | MB_OK);
        return 1;
    }

    // Position the window at the bottom-right corner.
    RECT desktopRect;
    GetWindowRect(GetDesktopWindow(), &desktopRect);
    int xOffset = 5;
    int yOffset = 80;
    int windowHeight = static_cast<int>(desktopRect.bottom * 0.7);
    int windowWidth = 400;
    int x = desktopRect.right - windowWidth - xOffset;
    int y = desktopRect.bottom - windowHeight - yOffset;
    MoveWindow(hwnd, x, y, windowWidth, windowHeight, TRUE);
    SetWindowPos(hwnd, HWND_TOPMOST, x, y, windowWidth, windowHeight, SWP_SHOWWINDOW);

    // Make the window unresizable and remove the maximize button.
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    dwStyle &= ~WS_SIZEBOX;
    dwStyle &= ~WS_MAXIMIZEBOX;
    SetWindowLong(hwnd, GWL_STYLE, dwStyle);

    // Apply a custom font.
    HFONT hFont = CreateFont(
        20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"AfacadFlux-Regular");
    if (hFont) {
        SendMessage(hwnd, WM_SETFONT, WPARAM(hFont), TRUE);
    }

    // Run the message loop.
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hFont) {
        DeleteObject(hFont);
    }

    return static_cast<int>(msg.wParam);
}
