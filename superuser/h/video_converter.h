#ifndef VIDEO_CONVERTER_H
#define VIDEO_CONVERTER_H

#include <windows.h> // For HWND
#include <string>    // For std::wstring

#ifdef __cplusplus
extern "C" {
#endif

void convert_video(HWND hwnd, const std::wstring& inputFilePath, int threadCount);

#ifdef __cplusplus
}
#endif

#endif // VIDEO_CONVERTER_H