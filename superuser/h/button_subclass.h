#ifndef BUTTON_SUBCLASS_H
#define BUTTON_SUBCLASS_H

#include <windows.h>

// Declare the global WNDPROC variables without initialization (no '=' sign)
extern WNDPROC g_originalButtonProcUpdate;
extern WNDPROC g_originalButtonProcCopy;
extern WNDPROC g_originalButtonProcCompress;
extern WNDPROC g_originalButtonProcDecompress;
extern WNDPROC g_originalButtonProcAttach;
extern WNDPROC g_originalButtonProcPrev;
extern WNDPROC g_originalButtonProcNext;
extern WNDPROC g_OrigListBoxProc; 
extern WNDPROC g_OrigFrameProc;
extern WNDPROC g_originalButtonProcDelete;

bool HandleButtonPaint(HWND hwnd, HDC hdc, RECT rect);


// Function declaration for the button subclass procedure
LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // BUTTON_SUBCLASS_H
