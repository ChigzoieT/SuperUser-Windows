#ifndef PRESSORFILE_H
#define PRESSORFILE_H

#include <string>
#include <lzma.h>
#include "superuser.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototype for compressing a file with its extension embedded.
// Matches the parent's C++ function signature: takes an HWND, an input file path, and a thread count.
void compressFileWithExtension(HWND hwnd, const std::wstring& inputFilePath, int numThreads);

void validateclickz(HWND hwnd, const wchar_t* text, COLORREF color);
void CALLBACK ClearText(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void validateclickzWithTimer(HWND hwnd, const wchar_t* text, COLORREF color, UINT delay);

#ifdef __cplusplus
}
#endif

#endif // PRESSORFILE_H
