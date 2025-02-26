#include "../h/audio_converter.h"
#include "../h/pressorfile.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <codecvt>
#include <windows.h>
#include <filesystem> // For directory handling

#ifdef __cplusplus
extern "C" {
#endif

// Helper function: Convert std::wstring to std::string (UTF-8)
std::string wstring_to_utf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Helper function: Derive output filename by changing extension to ".aac"
std::wstring get_output_filename(const std::wstring& inputFilePath) {
    size_t dotPos = inputFilePath.find_last_of(L".");
    return (dotPos != std::wstring::npos) ? inputFilePath.substr(0, dotPos) + L".aac" : inputFilePath + L".aac";
}

void convert_audio(HWND hwnd, const std::wstring& inputFilePath, int numThreads) {
    int ret = 0;

    AVFormatContext* in_format_ctx = nullptr;
    AVFormatContext* out_format_ctx = nullptr;
    AVCodecContext* in_codec_ctx = nullptr;
    AVCodecContext* out_codec_ctx = nullptr;
    const AVCodec* in_codec = nullptr;
    const AVCodec* out_codec = nullptr;
    AVStream* in_stream = nullptr;
    AVStream* out_stream = nullptr;
    SwrContext* swr_ctx = nullptr;
    AVFrame* in_frame = nullptr;
    AVFrame* out_frame = nullptr;
    AVPacket* pkt = nullptr;
    int stream_index = -1;

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
    std::wstring outputFilePath = (outputDir / inputPath.stem()).wstring() + L".aac";
    std::string output_filename = wstring_to_utf8(outputFilePath);

    // Initialize libavformat and register all formats and codecs
    avformat_network_init();

    // Open input file
    if ((ret = avformat_open_input(&in_format_ctx, wstring_to_utf8(inputFilePath).c_str(), nullptr, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Retrieve input stream information
    if ((ret = avformat_find_stream_info(in_format_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Find the audio stream
    for (unsigned int i = 0; i < in_format_ctx->nb_streams; i++) {
        if (in_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_index = i;
            break;
        }
    }
    if (stream_index < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    in_stream = in_format_ctx->streams[stream_index];

    // Calculate the total duration of the input file in seconds
    total_duration = in_format_ctx->duration; // Duration in AV_TIME_BASE units
    total_duration_seconds = (double)total_duration / AV_TIME_BASE;

    // Find the decoder for the audio stream
    in_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
    if (!in_codec) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate a codec context for the decoder
    in_codec_ctx = avcodec_alloc_context3(in_codec);
    if (!in_codec_ctx) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Copy codec parameters from input stream to codec context
    if ((ret = avcodec_parameters_to_context(in_codec_ctx, in_stream->codecpar)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Set the number of threads for decoding
    in_codec_ctx->thread_count = numThreads;

    // Open the decoder
    if ((ret = avcodec_open2(in_codec_ctx, in_codec, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate the output format context
    if ((ret = avformat_alloc_output_context2(&out_format_ctx, nullptr, nullptr, output_filename.c_str())) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Create a new audio stream in the output file
    out_stream = avformat_new_stream(out_format_ctx, nullptr);
    if (!out_stream) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Find the encoder for the output format
    out_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!out_codec) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate a codec context for the encoder
    out_codec_ctx = avcodec_alloc_context3(out_codec);
    if (!out_codec_ctx) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Set encoder parameters
    out_codec_ctx->bit_rate = 64000;
    out_codec_ctx->sample_rate = in_codec_ctx->sample_rate;
    out_codec_ctx->ch_layout = in_codec_ctx->ch_layout;
    out_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP; // Use a default sample format
    out_codec_ctx->thread_count = numThreads;

    // Open the encoder
    if ((ret = avcodec_open2(out_codec_ctx, out_codec, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Copy encoder parameters to the output stream
    if ((ret = avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Open the output file
    if (!(out_format_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&out_format_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE)) < 0) {
            validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    // Write the header to the output file
    if ((ret = avformat_write_header(out_format_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Initialize the resampler if needed
    if (in_codec_ctx->sample_fmt != out_codec_ctx->sample_fmt ||
        in_codec_ctx->sample_rate != out_codec_ctx->sample_rate ||
        av_channel_layout_compare(&in_codec_ctx->ch_layout, &out_codec_ctx->ch_layout) != 0) {

        swr_ctx = swr_alloc();
        if (!swr_ctx) {
            validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        av_opt_set_chlayout(swr_ctx, "in_chlayout", &in_codec_ctx->ch_layout, 0);
        av_opt_set_int(swr_ctx, "in_sample_rate", in_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", in_codec_ctx->sample_fmt, 0);

        av_opt_set_chlayout(swr_ctx, "out_chlayout", &out_codec_ctx->ch_layout, 0);
        av_opt_set_int(swr_ctx, "out_sample_rate", out_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", out_codec_ctx->sample_fmt, 0);

        if ((ret = swr_init(swr_ctx)) < 0) {
            validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    // Allocate frames and packet
    in_frame = av_frame_alloc();
    out_frame = av_frame_alloc();
    pkt = av_packet_alloc();
    if (!in_frame || !out_frame || !pkt) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Read frames from the input file, process them, and write to the output file
    while ((ret = av_read_frame(in_format_ctx, pkt)) >= 0) {
        if (pkt->stream_index != stream_index) {
            av_packet_unref(pkt);
            continue;
        }

        // Calculate progress in 0.1% increments
        if (total_duration > 0) {
            double current_time = (double)pkt->pts * av_q2d(in_stream->time_base);
            conversion_progress = (current_time / total_duration_seconds) * 100.0;
            if (conversion_progress > 100.0) conversion_progress = 100.0; // Clamp to 100%

            // Pass the progress to validateclickz with blue color
            wchar_t progress_text[64];
            swprintf(progress_text, 64, L"Progress: %.1f%%", conversion_progress);
            validateclickz(hwnd, progress_text, RGB(0, 0, 255)); // Blue color for progress
        }

        ret = avcodec_send_packet(in_codec_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(in_codec_ctx, in_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }

            if (swr_ctx) {
                out_frame->sample_rate = out_codec_ctx->sample_rate;
                out_frame->ch_layout = out_codec_ctx->ch_layout;
                out_frame->format = out_codec_ctx->sample_fmt;
                out_frame->nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, in_codec_ctx->sample_rate) + in_frame->nb_samples,
                                                      out_codec_ctx->sample_rate, in_codec_ctx->sample_rate, AV_ROUND_UP);
                if ((ret = av_frame_get_buffer(out_frame, 0)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
                if ((ret = swr_convert(swr_ctx, out_frame->data, out_frame->nb_samples, (const uint8_t**)in_frame->data, in_frame->nb_samples)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            } else {
                if ((ret = av_frame_ref(out_frame, in_frame)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            }

            ret = avcodec_send_frame(out_codec_ctx, out_frame);
            av_frame_unref(out_frame);
            av_frame_unref(in_frame);
            if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }

            while (ret >= 0) {
                AVPacket* enc_pkt = av_packet_alloc();
                ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_packet_free(&enc_pkt);
                    break;
                } else if (ret < 0) {
                    av_packet_free(&enc_pkt);
                    validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }

                enc_pkt->stream_index = out_stream->index;
                av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

                ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
                av_packet_free(&enc_pkt);
                if (ret < 0) {
                    validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            }
        }
    }

    // Flush the encoder
    ret = avcodec_send_frame(out_codec_ctx, nullptr);
    if (ret < 0 && ret != AVERROR_EOF) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    while (true) {
        AVPacket* enc_pkt = av_packet_alloc();
        ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
        if (ret == AVERROR_EOF) {
            av_packet_free(&enc_pkt);
            break;
        } else if (ret < 0) {
            av_packet_free(&enc_pkt);
            validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        enc_pkt->stream_index = out_stream->index;
        av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

        ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
        av_packet_free(&enc_pkt);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    // Write the trailer to the output file
    if ((ret = av_write_trailer(out_format_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    validateclickzWithTimer(hwnd, L"Conversion Complete!", RGB(0, 255, 0), 3000);

cleanup:
    if (in_frame) av_frame_free(&in_frame);
    if (out_frame) av_frame_free(&out_frame);
    if (pkt) av_packet_free(&pkt);
    if (swr_ctx) swr_free(&swr_ctx);
    if (in_codec_ctx) avcodec_free_context(&in_codec_ctx);
    if (out_codec_ctx) avcodec_free_context(&out_codec_ctx);
    if (in_format_ctx) avformat_close_input(&in_format_ctx);
    if (out_format_ctx) {
        if (!(out_format_ctx->oformat->flags & AVFMT_NOFILE))
            avio_closep(&out_format_ctx->pb);
        avformat_free_context(out_format_ctx);
    }
}

#ifdef __cplusplus
} // Close the extern "C" block
#endif