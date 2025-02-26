#include "../h/prevnext.h"
#include "../h/button_subclass.h"
#include "../h/superuser.h"
#include "../h/components.h"
#include <windows.h>
#include <algorithm>  // for std::min
// Helper function to make the button circular
/*void MakeCircularButton(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int diameter = std::min(width, height); // Use std::min explicitly

    // Make the button square for circular shape
    SetWindowPos(hwnd, NULL, rect.left, rect.top, diameter, diameter, SWP_NOZORDER);
}*/

// Callback for the Previous Button
LRESULT CALLBACK PrevButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            int radius = (rect.right - rect.left) / 2;
            int centerX = rect.left + radius;
            int centerY = rect.top + radius;

            // Set up drawing
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            
            // Draw a circle
            Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);

            // Draw the text (<) centered
            DrawText(hdc, L"<", 1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SETCURSOR: {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE;
        }

    default:
        break;
    }

    //Handle custom button procedure for the previous button
    if (hwnd == hButtonPrev && g_originalButtonProcPrev) {
        return CallWindowProc(g_originalButtonProcPrev, hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Callback for the Next Button
LRESULT CALLBACK NextButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            int radius = (rect.right - rect.left) / 2;
            int centerX = rect.left + radius;
            int centerY = rect.top + radius;

            // Set up drawing
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));

            // Draw a circle
            Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);

            // Draw the text (>) centered
            DrawText(hdc, L">", 1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SETCURSOR: {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE;
        }

    default:
        break;
    }

    // Handle custom button procedure for the next button
    if (hwnd == hButtonNext && g_originalButtonProcNext) {
        return CallWindowProc(g_originalButtonProcNext, hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}