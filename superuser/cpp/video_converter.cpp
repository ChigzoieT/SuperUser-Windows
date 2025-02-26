#include "../h/video_converter.h"
#include "../h/pressorfile.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <codecvt>
#include <windows.h>
#include <filesystem> // For directory handling

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// Helper function: Convert std::wstring to std::string (UTF-8)
std::string wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Helper function: Derive output filename by changing extension to ".mp4"
std::wstring get_output_filename(const std::wstring& inputFilePath) {
    size_t dotPos = inputFilePath.find_last_of(L".");
    return (dotPos != std::wstring::npos) ? inputFilePath.substr(0, dotPos) + L".mp4" : inputFilePath + L".mp4";
}

void convert_video(HWND hwnd, const std::wstring& inputFilePath, int threadCount) {
    int ret = 0;

    AVFormatContext* in_fmt_ctx = nullptr;
    AVFormatContext* out_fmt_ctx = nullptr;
    AVCodecContext* dec_ctx = nullptr;
    AVCodecContext* enc_ctx = nullptr;
    const AVCodec* decoder = nullptr;
    const AVCodec* encoder = nullptr;
    AVStream* in_video_stream = nullptr;
    AVStream* out_stream = nullptr;
    SwsContext* sws_ctx = nullptr;
    AVFrame* frame_decoded = nullptr;
    AVFrame* frame_converted = nullptr;
    AVPacket* packet_in = nullptr;
    AVPacket* packet_out = nullptr;

    // Declare video_stream_index at the top to avoid goto issues
    int video_stream_index = -1;

    // Variable to store conversion progress in 0.1% increments
    double conversion_progress = 0.0;

    // Variables for total duration and progress calculation
    int64_t total_duration = 0;
    double total_duration_seconds = 0.0;

    if (inputFilePath.empty()) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return; // Early return on error
    }

    // Get the directory of the input file
    std::filesystem::path inputPath(inputFilePath);
    std::filesystem::path outputDir = inputPath.parent_path() / "compressed";

    // Create the "compressed" directory if it doesn't exist
    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directory(outputDir);
    }

    // Derive the output file path in the "compressed" directory
    std::wstring outputFilePath = (outputDir / inputPath.stem()).wstring() + L".mp4";
    std::string output_filename = wstring_to_utf8(outputFilePath);

    // Initialize libavformat and register all formats and codecs
    avformat_network_init();

    // Open input file
    if ((ret = avformat_open_input(&in_fmt_ctx, wstring_to_utf8(inputFilePath).c_str(), nullptr, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Retrieve input stream information
    if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Find the best video stream
    video_stream_index = av_find_best_stream(in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_index < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    in_video_stream = in_fmt_ctx->streams[video_stream_index];

    // Calculate the total duration of the input file in seconds
    total_duration = in_fmt_ctx->duration; // Duration in AV_TIME_BASE units
    total_duration_seconds = (double)total_duration / AV_TIME_BASE;

    // Find the decoder for the video stream
    decoder = avcodec_find_decoder(in_video_stream->codecpar->codec_id);
    if (!decoder) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate a codec context for the decoder
    dec_ctx = avcodec_alloc_context3(decoder);
    if (!dec_ctx) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Copy codec parameters from input stream to codec context
    if ((ret = avcodec_parameters_to_context(dec_ctx, in_video_stream->codecpar)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Set the number of threads for decoding
    dec_ctx->thread_count = threadCount;

    // Open the decoder
    if ((ret = avcodec_open2(dec_ctx, decoder, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate the output format context (using MP4 container)
    if ((ret = avformat_alloc_output_context2(&out_fmt_ctx, nullptr, "mp4", output_filename.c_str())) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Find the H.265 encoder (HEVC)
    encoder = avcodec_find_encoder(AV_CODEC_ID_HEVC);
    if (!encoder) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Create a new video stream in the output file
    out_stream = avformat_new_stream(out_fmt_ctx, nullptr);
    if (!out_stream) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate and configure the encoder context
    enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Set encoder parameters
    enc_ctx->height = dec_ctx->height;
    enc_ctx->width = dec_ctx->width;
    enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P; // Use YUV420P pixel format
    enc_ctx->time_base = av_inv_q(dec_ctx->framerate.num ? dec_ctx->framerate : in_video_stream->r_frame_rate);
    enc_ctx->thread_count = threadCount;

    // Set preset options if supported
    av_opt_set(enc_ctx->priv_data, "preset", "medium", 0);

    // Open the encoder
    if ((ret = avcodec_open2(enc_ctx, encoder, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Copy encoder parameters to the output stream
    if ((ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    out_stream->time_base = enc_ctx->time_base;

    // Open the output file if needed
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&out_fmt_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE)) < 0) {
            validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    // Write the stream header to the output file
    if ((ret = avformat_write_header(out_fmt_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate frames and packets for conversion
    frame_decoded = av_frame_alloc();
    frame_converted = av_frame_alloc();
    packet_in = av_packet_alloc();
    packet_out = av_packet_alloc();
    if (!frame_decoded || !frame_converted || !packet_in || !packet_out) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Prepare a scaler if the decoder pixel format isn't YUV420P
    sws_ctx = sws_getContext(dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
                             enc_ctx->width, enc_ctx->height, enc_ctx->pix_fmt,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate buffer for the converted frame
    ret = av_image_alloc(frame_converted->data, frame_converted->linesize,
                         enc_ctx->width, enc_ctx->height, enc_ctx->pix_fmt, 32);
    if (ret < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    frame_converted->width = enc_ctx->width;
    frame_converted->height = enc_ctx->height;
    frame_converted->format = enc_ctx->pix_fmt;
    frame_converted->pts = 0;

    // Main conversion loop: read, decode, convert, encode, and write
    while ((ret = av_read_frame(in_fmt_ctx, packet_in)) >= 0) {
        if (packet_in->stream_index == video_stream_index) {
            // Calculate progress in 0.1% increments
            if (total_duration > 0) {
                double current_time = (double)packet_in->pts * av_q2d(in_video_stream->time_base);
                conversion_progress = (current_time / total_duration_seconds) * 100.0;
                if (conversion_progress > 100.0) conversion_progress = 100.0; // Clamp to 100%

                // Pass the progress to validateclickz with blue color
                wchar_t progress_text[64];
                swprintf(progress_text, 64, L"Progress: %.1f%%", conversion_progress);
                validateclickz(hwnd, progress_text, RGB(0, 0, 255)); // Blue color for progress
            }

            ret = avcodec_send_packet(dec_ctx, packet_in);
            if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                break;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame_decoded);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }

                // Convert the frame to the encoder's pixel format
                sws_scale(sws_ctx, frame_decoded->data, frame_decoded->linesize, 0, dec_ctx->height,
                          frame_converted->data, frame_converted->linesize);
                frame_converted->pts = frame_decoded->pts;

                // Encode the frame
                ret = avcodec_send_frame(enc_ctx, frame_converted);
                if (ret < 0) {
                    validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
                while (ret >= 0) {
                    ret = avcodec_receive_packet(enc_ctx, packet_out);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    else if (ret < 0) {
                        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                        goto cleanup;
                    }
                    // Rescale packet timestamp
                    av_packet_rescale_ts(packet_out, enc_ctx->time_base, out_stream->time_base);
                    packet_out->stream_index = out_stream->index;
                    // Write packet
                    if ((ret = av_interleaved_write_frame(out_fmt_ctx, packet_out)) < 0) {
                        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                        goto cleanup;
                    }
                    av_packet_unref(packet_out);
                }
                av_frame_unref(frame_decoded);
            }
        }
        av_packet_unref(packet_in);
    }

    // Write trailer to output file
    if ((ret = av_write_trailer(out_fmt_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    validateclickzWithTimer(hwnd, L"Conversion Complete!", RGB(0, 255, 0), 3000);

cleanup:
    if (sws_ctx) sws_freeContext(sws_ctx);
    if (frame_converted) av_freep(&frame_converted->data[0]);
    if (frame_decoded) av_frame_free(&frame_decoded);
    if (frame_converted) av_frame_free(&frame_converted);
    if (packet_in) av_packet_free(&packet_in);
    if (packet_out) av_packet_free(&packet_out);
    if (out_fmt_ctx && !(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&out_fmt_ctx->pb);
    if (enc_ctx) avcodec_free_context(&enc_ctx);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (in_fmt_ctx) avformat_close_input(&in_fmt_ctx);
    if (out_fmt_ctx) avformat_free_context(out_fmt_ctx);
}