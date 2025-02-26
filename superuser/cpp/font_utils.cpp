#include "h/font_utils.h"
#include "h/resource.h"
#include <windows.h>
#include <commdlg.h>

HFONT LoadCustomFont() {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_FONT1), RT_RCDATA);
    if (hRes) {
        HGLOBAL hData = LoadResource(NULL, hRes);
        if (hData) {
            void* pData = LockResource(hData);
            DWORD dwSize = SizeofResource(NULL, hRes);

            DWORD fontsAdded = 0;
            HANDLE hFont = AddFontMemResourceEx(pData, dwSize, NULL, &fontsAdded);
            if (hFont && fontsAdded > 0) {
                HFONT hFontHandle = CreateFont(
                    19, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Afacad Flux");
                if (hFontHandle) {
                    return hFontHandle;
                }
            }
        }
    }
    return (HFONT)GetStockObject(SYSTEM_FONT);
}


HFONT LoadRootFont() {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_FONT1), RT_RCDATA);
    if (hRes) {
        HGLOBAL hData = LoadResource(NULL, hRes);
        if (hData) {
            void* pData = LockResource(hData);
            DWORD dwSize = SizeofResource(NULL, hRes);

            DWORD fontsAdded = 0;
            HANDLE hFont = AddFontMemResourceEx(pData, dwSize, NULL, &fontsAdded);
            if (hFont && fontsAdded > 0) {
                HFONT hFontHandle = CreateFont(
                    19, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Afacad Flux");
                if (hFontHandle) {
                    return hFontHandle;
                }
            }
        }
    }
    return (HFONT)GetStockObject(SYSTEM_FONT);
}

