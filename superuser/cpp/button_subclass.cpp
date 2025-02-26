#include <windows.h>
#include "../h/font_utils.h"
#include "../h/superuser.h"
#include "../h/button_subclass.h"
#include <wchar.h>
#include "../h/resource.h"

// Hover state for buttons
static bool isHoveredUpdate = false;
static bool isHoveredCopy = false;
static bool isHoveredCompress = false;
static bool isHoveredDecompress = false;
static bool isHoveredAttach = false;
static bool isHoveredDelete = false;

LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SETCURSOR: {
            SetCursor(LoadCursor(nullptr, IDC_HAND));
            return TRUE;
        }

        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);

            RECT rect;
            GetClientRect(hwnd, &rect);
            POINT mousePos;
            GetCursorPos(&mousePos);
            ScreenToClient(hwnd, &mousePos);

            bool isCurrentlyHovered = PtInRect(&rect, mousePos);

            if (hwnd == hButtonUpdate && isHoveredUpdate != isCurrentlyHovered) {
                isHoveredUpdate = isCurrentlyHovered;
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (hwnd == hButtonCopy && isHoveredCopy != isCurrentlyHovered) {
                isHoveredCopy = isCurrentlyHovered;
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (hwnd == hButtonCompress && isHoveredCompress != isCurrentlyHovered) {
                isHoveredCompress = isCurrentlyHovered;
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (hwnd == hButtonDecompress && isHoveredDecompress != isCurrentlyHovered) {
                isHoveredDecompress = isCurrentlyHovered;
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (hwnd == hButtonAttach && isHoveredAttach != isCurrentlyHovered) {
                isHoveredAttach = isCurrentlyHovered;
                InvalidateRect(hwnd, nullptr, TRUE);
            } else if (hwnd == hButtonDelete && isHoveredDelete != isCurrentlyHovered) {
                isHoveredDelete = isCurrentlyHovered;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        }

        case WM_MOUSELEAVE: {
            if (hwnd == hButtonUpdate) isHoveredUpdate = false;
            else if (hwnd == hButtonCopy) isHoveredCopy = false;
            else if (hwnd == hButtonCompress) isHoveredCompress = false;
            else if (hwnd == hButtonDecompress) isHoveredDecompress = false;
            else if (hwnd == hButtonAttach) isHoveredAttach = false;
            else if (hwnd == hButtonDelete) isHoveredDelete = false;

            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);

            if (HandleButtonPaint(hwnd, hdc, rect)) {
                EndPaint(hwnd, &ps);
                return 0;
            }

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_ERASEBKGND:
        case WM_NCPAINT:
            return 1;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            InvalidateRect(hwnd, nullptr, TRUE);
            break;

        default:
            break;
    }

    // Call the original window procedure for the button
    if (hwnd == hButtonUpdate && g_originalButtonProcUpdate) {
        return CallWindowProc(g_originalButtonProcUpdate, hwnd, uMsg, wParam, lParam);
    } else if (hwnd == hButtonCopy && g_originalButtonProcCopy) {
        return CallWindowProc(g_originalButtonProcCopy, hwnd, uMsg, wParam, lParam);
    } else if (hwnd == hButtonCompress && g_originalButtonProcCompress) {
        return CallWindowProc(g_originalButtonProcCompress, hwnd, uMsg, wParam, lParam);
    } else if (hwnd == hButtonDecompress && g_originalButtonProcDecompress) {
        return CallWindowProc(g_originalButtonProcDecompress, hwnd, uMsg, wParam, lParam);
    } else if (hwnd == hButtonAttach && g_originalButtonProcAttach) {
        return CallWindowProc(g_originalButtonProcAttach, hwnd, uMsg, wParam, lParam);
    } else if (hwnd == hButtonDelete && g_originalButtonProcDelete) {
        return CallWindowProc(g_originalButtonProcDelete, hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool HandleButtonPaint(HWND hwnd, HDC hdc, RECT rect) {
    // Obtain the instance handle.
    HINSTANCE hInst = GetModuleHandle(NULL);

    HFONT hFont = LoadCustomFont();
    if (!hFont)
        return false;

    // Determine background color based on hover state.
    COLORREF bgColor = RGB(255, 255, 255);
    int borderRadius = 5;
    if ((hwnd == hButtonUpdate && isHoveredUpdate) ||
        (hwnd == hButtonCopy   && isHoveredCopy)   ||
        (hwnd == hButtonCompress && isHoveredCompress) ||
        (hwnd == hButtonDecompress && isHoveredDecompress) ||
        (hwnd == hButtonAttach && isHoveredAttach) ||
        (hwnd == hButtonDelete && isHoveredDelete))
    {
        bgColor = RGB(135, 206, 235);
    }

    HBRUSH hBrush = CreateSolidBrush(bgColor);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

    // Select the GDI objects into the device context.
    SelectObject(hdc, hBrush);
    SelectObject(hdc, hPen);
    SelectObject(hdc, hFont);

    // Draw a rounded rectangle for the button background.
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, borderRadius, borderRadius);
    SetBkMode(hdc, TRANSPARENT);

    // For buttons that should display an icon (Copy, Update, Attach File, Delete)
    if (hwnd == hButtonCopy || hwnd == hButtonUpdate || hwnd == hButtonAttach || hwnd == hButtonDelete) {
        HICON hIcon = nullptr;
        if (hwnd == hButtonCopy) {
            hIcon = (HICON)LoadImage(
                hInst,
                MAKEINTRESOURCE(IDI_COPY),
                IMAGE_ICON,
                19, 19,
                LR_DEFAULTCOLOR
            );
        } else if (hwnd == hButtonUpdate) {
            hIcon = (HICON)LoadImage(
                hInst,
                MAKEINTRESOURCE(IDI_UPLOAD),
                IMAGE_ICON,
                19, 19,
                LR_DEFAULTCOLOR
            );
        } else if (hwnd == hButtonAttach) {
            hIcon = (HICON)LoadImage(
                hInst,
                MAKEINTRESOURCE(IDI_FILE),
                IMAGE_ICON,
                19, 19,
                LR_DEFAULTCOLOR
            );
        } else if (hwnd == hButtonDelete) {
            hIcon = (HICON)LoadImage(
                hInst,
                MAKEINTRESOURCE(IDI_DELETE),
                IMAGE_ICON,
                19, 19,
                LR_DEFAULTCOLOR
            );
        }

        if (hIcon) {
            int iconWidth = 19;
            int iconHeight = 19;
            int x = rect.left + ((rect.right - rect.left) - iconWidth) / 2;
            int y = rect.top + ((rect.bottom - rect.top) - iconHeight) / 2;
            DrawIconEx(hdc, x, y, hIcon, iconWidth, iconHeight, 0, NULL, DI_NORMAL);
            DestroyIcon(hIcon);
        }
    }
    else {
        // For text-based buttons (Compress and Decompress).
        const wchar_t* buttonText = nullptr;
        if (hwnd == hButtonCompress)
            buttonText = L"Compress";
        else if (hwnd == hButtonDecompress)
            buttonText = L"Decompress";

        if (buttonText) {
            DrawText(hdc, buttonText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    // Clean up GDI objects.
    DeleteObject(hBrush);
    DeleteObject(hPen);
    DeleteObject(hFont);

    return true;
}
