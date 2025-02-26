#ifndef IMAGE_CONVERTER_H
#define IMAGE_CONVERTER_H

#include <string>
#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif

// Converts an image (in any supported format) to a JPEG image and saves it in a "compressed" folder.
// input_file - path to the source image file (e.g., PNG, BMP, TIFF, etc.).
void convert_image(HWND hwnd, const std::wstring& input_file, int thread_count);


#ifdef __cplusplus
}
#endif

#endif // IMAGE_CONVERTER_H
