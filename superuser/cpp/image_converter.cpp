#include "../h/image_converter.h"
#include "../h/pressorfile.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
}

#include <cstdio>
#include <filesystem>
#include <string>
#include <windows.h>

// Function to convert image and show progress
void convert_image(HWND hwnd, const std::wstring& input_file, int thread_count) {
    std::string inputFileStr(input_file.begin(), input_file.end());

    std::filesystem::path inputPath(input_file);
    std::filesystem::path outputDir = inputPath.parent_path() / "compressed";
    std::filesystem::create_directories(outputDir);

    std::wstring outputFileW = outputDir / (inputPath.stem().wstring() + L".jpg");
    std::string outputFileStr(outputFileW.begin(), outputFileW.end());

    AVFormatContext* format_ctx = nullptr;
    if (avformat_open_input(&format_ctx, inputFileStr.c_str(), nullptr, nullptr) != 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return;
    }

    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        avformat_close_input(&format_ctx);
        return;
    }

    int stream_index = -1;
    for (unsigned i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream_index = i;
            break;
        }
    }
    if (stream_index == -1) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        avformat_close_input(&format_ctx);
        return;
    }

    AVCodecParameters* codec_params = format_ctx->streams[stream_index]->codecpar;
    const AVCodec* decoder = avcodec_find_decoder(codec_params->codec_id);
    if (!decoder) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        avformat_close_input(&format_ctx);
        return;
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(codec_ctx, codec_params);
    codec_ctx->thread_count = thread_count;
    avcodec_open2(codec_ctx, decoder, nullptr);

    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();

    int totalFrames = format_ctx->streams[stream_index]->nb_frames;
    int frameCount = 0;

    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == stream_index && avcodec_send_packet(codec_ctx, packet) == 0) {
            while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                frameCount++;

                // Calculate progress: range 0.1% to 100%
                double progress = ((double)frameCount / totalFrames) * 99.9 + 0.1;
                wchar_t progressText[50];
                swprintf(progressText, 50, L"Compression Progress: %.1f%%", progress);
                validateclickz(hwnd, progressText, RGB(0, 255, 0));

                struct SwsContext* sws_ctx = sws_getContext(
                    codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                    codec_ctx->width, codec_ctx->height, AV_PIX_FMT_YUVJ420P,
                    SWS_BICUBIC, nullptr, nullptr, nullptr);
                
                AVFrame* yuv_frame = av_frame_alloc();
                int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUVJ420P, codec_ctx->width, codec_ctx->height, 1);
                uint8_t* buffer = (uint8_t*)av_malloc(buffer_size);
                av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize, buffer,
                                     AV_PIX_FMT_YUVJ420P, codec_ctx->width, codec_ctx->height, 1);
                
                sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height,
                          yuv_frame->data, yuv_frame->linesize);
                
                const AVCodec* jpeg_codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
                AVCodecContext* jpeg_ctx = avcodec_alloc_context3(jpeg_codec);
                jpeg_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
                jpeg_ctx->height = codec_ctx->height;
                jpeg_ctx->width = codec_ctx->width;
                jpeg_ctx->time_base = (AVRational){1, 25};
                jpeg_ctx->max_b_frames = 1;
                jpeg_ctx->gop_size = 10;
                jpeg_ctx->thread_count = thread_count;
                jpeg_ctx->flags |= AV_CODEC_FLAG_QSCALE;
                jpeg_ctx->global_quality = FF_QP2LAMBDA * 5; // Use qscale=5
                avcodec_open2(jpeg_ctx, jpeg_codec, nullptr);
                
                AVPacket* out_packet = av_packet_alloc();
                if (avcodec_send_frame(jpeg_ctx, yuv_frame) == 0 && avcodec_receive_packet(jpeg_ctx, out_packet) == 0) {
                    FILE* file = fopen(outputFileStr.c_str(), "wb");
                    if (file) {
                        fwrite(out_packet->data, 1, out_packet->size, file);
                        fclose(file);
                    }
                    av_packet_unref(out_packet);
                }

                av_packet_free(&out_packet);
                avcodec_free_context(&jpeg_ctx);
                av_free(buffer);
                av_frame_free(&yuv_frame);
                sws_freeContext(sws_ctx);
            }
        }
        av_packet_unref(packet);
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    // Compression complete message (Green, 5 seconds)
    validateclickzWithTimer(hwnd, L"Image Compression Complete!", RGB(0, 255, 0), 5000);
}
