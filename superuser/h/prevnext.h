#pragma once
#include <windows.h>

// Function declarations for the Prev and Next buttons
LRESULT CALLBACK PrevButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK NextButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
