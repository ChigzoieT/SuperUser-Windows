#ifndef AUDIO_CONVERTER_H
#define AUDIO_CONVERTER_H

#include <string>


#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

// Sets the thread count for a given codec context.
void set_thread_count(AVCodecContext* codec_ctx, int thread_count);

// Converts the audio in the given input file to AAC and writes the output file automatically.
// Takes an HWND for GUI-related operations, a wide-string input file path, and a thread count.
// Returns 0 on success, non-zero on failure.
void convert_audio(HWND hwnd, const std::wstring& inputFilePath, int numThreads);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_CONVERTER_H
