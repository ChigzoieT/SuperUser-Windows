int convert_audio(HWND hwnd, const std::wstring& inputFilePath, int numThreads) {
    int ret = 0;

    AVFormatContext *in_format_ctx = nullptr;
    AVFormatContext *out_format_ctx = nullptr;
    AVCodecContext *in_codec_ctx = nullptr;
    AVCodecContext *out_codec_ctx = nullptr;
    const AVCodec *in_codec = nullptr;
    const AVCodec *out_codec = nullptr;
    AVStream *in_stream = nullptr;
    AVStream *out_stream = nullptr;
    SwrContext *swr_ctx = nullptr; // Use FFmpeg's SwrContext instead of libsoxr
    AVFrame *in_frame = nullptr;
    AVFrame *out_frame = nullptr;
    AVPacket *pkt = nullptr;
    int stream_index = -1;

    if (inputFilePath.empty()) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        return -1;
    }

    std::string input_filename = wstring_to_utf8(inputFilePath);
    std::string output_filename = wstring_to_utf8(get_output_filename(inputFilePath));

    // Open input file
    if ((ret = avformat_open_input(&in_format_ctx, input_filename.c_str(), nullptr, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    if ((ret = avformat_find_stream_info(in_format_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Find the first audio stream
    for (unsigned i = 0; i < in_format_ctx->nb_streams; i++) {
        if (in_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_index = i;
            break;
        }
    }
    if (stream_index < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(EINVAL);
        goto cleanup;
    }
    in_stream = in_format_ctx->streams[stream_index];

    // Find and open the decoder
    in_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
    if (!in_codec) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR_DECODER_NOT_FOUND;
        goto cleanup;
    }
    in_codec_ctx = avcodec_alloc_context3(in_codec);
    if (!in_codec_ctx) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }
    if ((ret = avcodec_parameters_to_context(in_codec_ctx, in_stream->codecpar)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    if ((ret = avcodec_open2(in_codec_ctx, in_codec, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Allocate output format context
    if ((ret = avformat_alloc_output_context2(&out_format_ctx, nullptr, nullptr, output_filename.c_str())) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Create output stream
    out_stream = avformat_new_stream(out_format_ctx, nullptr);
    if (!out_stream) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    // Find and open the encoder
    out_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!out_codec) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR_ENCODER_NOT_FOUND;
        goto cleanup;
    }
    out_codec_ctx = avcodec_alloc_context3(out_codec);
    if (!out_codec_ctx) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    out_codec_ctx->bit_rate = 64000;
    out_codec_ctx->sample_rate = in_codec_ctx->sample_rate;
    out_codec_ctx->ch_layout = in_codec_ctx->ch_layout;
    out_codec_ctx->sample_fmt = out_codec->sample_fmts ? out_codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;

    if ((ret = avcodec_open2(out_codec_ctx, out_codec, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    if ((ret = avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Open the output file
    if (!(out_format_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&out_format_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE)) < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    // Write header to the output file
    if ((ret = avformat_write_header(out_format_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Initialize resampler if needed
    if (in_codec_ctx->sample_fmt != out_codec_ctx->sample_fmt ||
        in_codec_ctx->sample_rate != out_codec_ctx->sample_rate ||
        av_channel_layout_compare(&in_codec_ctx->ch_layout, &out_codec_ctx->ch_layout) != 0) {

        swr_ctx = swr_alloc();
        if (!swr_ctx) {
            ret = AVERROR(ENOMEM);
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        av_opt_set_chlayout(swr_ctx, "in_chlayout", &in_codec_ctx->ch_layout, 0);
        av_opt_set_int(swr_ctx, "in_sample_rate", in_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", in_codec_ctx->sample_fmt, 0);

        av_opt_set_chlayout(swr_ctx, "out_chlayout", &out_codec_ctx->ch_layout, 0);
        av_opt_set_int(swr_ctx, "out_sample_rate", out_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", out_codec_ctx->sample_fmt, 0);

        if ((ret = swr_init(swr_ctx)) < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    // Allocate frames and packet
    in_frame = av_frame_alloc();
    out_frame = av_frame_alloc();
    pkt = av_packet_alloc();
    if (!in_frame || !out_frame || !pkt) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    // Processing loop
    while ((ret = av_read_frame(in_format_ctx, pkt)) >= 0) {
        if (pkt->stream_index != stream_index) {
            av_packet_unref(pkt);
            continue;
        }

        ret = avcodec_send_packet(in_codec_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(in_codec_ctx, in_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }

            // Resample if needed
            if (swr_ctx) {
                out_frame->sample_rate = out_codec_ctx->sample_rate;
                out_frame->ch_layout = out_codec_ctx->ch_layout;
                out_frame->format = out_codec_ctx->sample_fmt;
                out_frame->nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, in_codec_ctx->sample_rate) + in_frame->nb_samples,
                                                       out_codec_ctx->sample_rate, in_codec_ctx->sample_rate, AV_ROUND_UP);
                if ((ret = av_frame_get_buffer(out_frame, 0)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
                if ((ret = swr_convert(swr_ctx, out_frame->data, out_frame->nb_samples, (const uint8_t**)in_frame->data, in_frame->nb_samples)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            } else {
                if ((ret = av_frame_ref(out_frame, in_frame)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            }

            // Send frame to encoder
            ret = avcodec_send_frame(out_codec_ctx, out_frame);
            av_frame_unref(out_frame);
            av_frame_unref(in_frame);
            if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }

            // Receive packets from encoder
            while (ret >= 0) {
                AVPacket *enc_pkt = av_packet_alloc();
                ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_packet_free(&enc_pkt);
                    break;
                } else if (ret < 0) {
                    av_packet_free(&enc_pkt);
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }

                enc_pkt->stream_index = out_stream->index;
                av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

                ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
                av_packet_free(&enc_pkt);
                if (ret < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            }
        }
    }

    // Flush decoder
    ret = avcodec_send_packet(in_codec_ctx, nullptr);
    if (ret < 0 && ret != AVERROR_EOF) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    while (true) {
        ret = avcodec_receive_frame(in_codec_ctx, in_frame);
        if (ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        if (swr_ctx) {
            out_frame->sample_rate = out_codec_ctx->sample_rate;
            out_frame->ch_layout = out_codec_ctx->ch_layout;
            out_frame->format = out_codec_ctx->sample_fmt;
            out_frame->nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, in_codec_ctx->sample_rate) + in_frame->nb_samples,
                                                   out_codec_ctx->sample_rate, in_codec_ctx->sample_rate, AV_ROUND_UP);
            if ((ret = av_frame_get_buffer(out_frame, 0)) < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
            if ((ret = swr_convert(swr_ctx, out_frame->data, out_frame->nb_samples, (const uint8_t**)in_frame->data, in_frame->nb_samples)) < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
        } else {
            if ((ret = av_frame_ref(out_frame, in_frame)) < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
        }

        ret = avcodec_send_frame(out_codec_ctx, out_frame);
        av_frame_unref(out_frame);
        av_frame_unref(in_frame);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        while (ret >= 0) {
            AVPacket *enc_pkt = av_packet_alloc();
            ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                av_packet_free(&enc_pkt);
                break;
            } else if (ret < 0) {
                av_packet_free(&enc_pkt);
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;

            }

            enc_pkt->stream_index = out_stream->index;
            av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

            ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
            av_packet_free(&enc_pkt);
            if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
        }
    }

    // Flush encoder
    ret = avcodec_send_frame(out_codec_ctx, nullptr);
    if (ret < 0 && ret != AVERROR_EOF) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    while (true) {
        AVPacket *enc_pkt = av_packet_alloc();
        ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
        if (ret == AVERROR_EOF) {
            av_packet_free(&enc_pkt);
            break;
        } else if (ret < 0) {
            av_packet_free(&enc_pkt);
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        enc_pkt->stream_index = out_stream->index;
        av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

        ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
        av_packet_free(&enc_pkt);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    // Write trailer to the output file
    if ((ret = av_write_trailer(out_format_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    // Success
    validateclickzWithTimer(hwnd, L"Conversion Complete!", RGB(0, 255, 0), 3000);

cleanup:
    // Free resources
    if (in_frame) av_frame_free(&in_frame);
    if (out_frame) av_frame_free(&out_frame);
    if (pkt) av_packet_free(&pkt);
    if (swr_ctx) swr_free(&swr_ctx);
    if (in_codec_ctx) avcodec_free_context(&in_codec_ctx);
    if (out_codec_ctx) avcodec_free_context(&out_codec_ctx);
    if (in_format_ctx) avformat_close_input(&in_format_ctx);
    if (out_format_ctx && !(out_format_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&out_format_ctx->pb);
    if (out_format_ctx) avformat_free_context(out_format_ctx);

    return ret;
}







int convert_audio(HWND hwnd, const std::wstring& inputFilePath, int numThreads) {
    int ret = 0;

    AVFormatContext *in_format_ctx = nullptr;
    AVFormatContext *out_format_ctx = nullptr;
    AVCodecContext *in_codec_ctx = nullptr;
    AVCodecContext *out_codec_ctx = nullptr;
    const AVCodec *in_codec = nullptr;
    const AVCodec *out_codec = nullptr;
    AVStream *in_stream = nullptr;
    AVStream *out_stream = nullptr;
    SwrContext *swr_ctx = nullptr;
    AVFrame *in_frame = nullptr;
    AVFrame *out_frame = nullptr;
    AVPacket *pkt = nullptr;
    int stream_index = -1;

    if (inputFilePath.empty()) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        return -1;
    }

    std::string input_filename = wstring_to_utf8(inputFilePath);
    std::string output_filename = wstring_to_utf8(get_output_filename(inputFilePath));

    if ((ret = avformat_open_input(&in_format_ctx, input_filename.c_str(), nullptr, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    if ((ret = avformat_find_stream_info(in_format_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    for (unsigned i = 0; i < in_format_ctx->nb_streams; i++) {
        if (in_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_index = i;
            break;
        }
    }
    if (stream_index < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(EINVAL);
        goto cleanup;
    }
    in_stream = in_format_ctx->streams[stream_index];

    in_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
    if (!in_codec) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR_DECODER_NOT_FOUND;
        goto cleanup;
    }
    in_codec_ctx = avcodec_alloc_context3(in_codec);
    if (!in_codec_ctx) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }
    if ((ret = avcodec_parameters_to_context(in_codec_ctx, in_stream->codecpar)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    in_codec_ctx->thread_count = numThreads;
    if ((ret = avcodec_open2(in_codec_ctx, in_codec, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    if ((ret = avformat_alloc_output_context2(&out_format_ctx, nullptr, nullptr, output_filename.c_str())) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    out_stream = avformat_new_stream(out_format_ctx, nullptr);
    if (!out_stream) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    out_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!out_codec) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR_ENCODER_NOT_FOUND;
        goto cleanup;
    }
    out_codec_ctx = avcodec_alloc_context3(out_codec);
    if (!out_codec_ctx) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    out_codec_ctx->bit_rate = 64000;
    out_codec_ctx->sample_rate = in_codec_ctx->sample_rate;
    out_codec_ctx->ch_layout = in_codec_ctx->ch_layout;
    out_codec_ctx->sample_fmt = out_codec->sample_fmts ? out_codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    out_codec_ctx->thread_count = numThreads;

    if ((ret = avcodec_open2(out_codec_ctx, out_codec, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }
    if ((ret = avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    if (!(out_format_ctx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&out_format_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE)) < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    if ((ret = avformat_write_header(out_format_ctx, nullptr)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    if (in_codec_ctx->sample_fmt != out_codec_ctx->sample_fmt ||
        in_codec_ctx->sample_rate != out_codec_ctx->sample_rate ||
        av_channel_layout_compare(&in_codec_ctx->ch_layout, &out_codec_ctx->ch_layout) != 0) {

        swr_ctx = swr_alloc();
        if (!swr_ctx) {
            ret = AVERROR(ENOMEM);
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        av_opt_set_chlayout(swr_ctx, "in_chlayout", &in_codec_ctx->ch_layout, 0);
        av_opt_set_int(swr_ctx, "in_sample_rate", in_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", in_codec_ctx->sample_fmt, 0);

        av_opt_set_chlayout(swr_ctx, "out_chlayout", &out_codec_ctx->ch_layout, 0);
        av_opt_set_int(swr_ctx, "out_sample_rate", out_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", out_codec_ctx->sample_fmt, 0);

        if ((ret = swr_init(swr_ctx)) < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    in_frame = av_frame_alloc();
    out_frame = av_frame_alloc();
    pkt = av_packet_alloc();
    if (!in_frame || !out_frame || !pkt) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        ret = AVERROR(ENOMEM);
        goto cleanup;
    }

    while ((ret = av_read_frame(in_format_ctx, pkt)) >= 0) {
        if (pkt->stream_index != stream_index) {
            av_packet_unref(pkt);
            continue;
        }

        ret = avcodec_send_packet(in_codec_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(in_codec_ctx, in_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }

            if (swr_ctx) {
                out_frame->sample_rate = out_codec_ctx->sample_rate;
                out_frame->ch_layout = out_codec_ctx->ch_layout;
                out_frame->format = out_codec_ctx->sample_fmt;
                out_frame->nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, in_codec_ctx->sample_rate) + in_frame->nb_samples,
                                                       out_codec_ctx->sample_rate, in_codec_ctx->sample_rate, AV_ROUND_UP);
                if ((ret = av_frame_get_buffer(out_frame, 0)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
                if ((ret = swr_convert(swr_ctx, out_frame->data, out_frame->nb_samples, (const uint8_t**)in_frame->data, in_frame->nb_samples)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            } else {
                if ((ret = av_frame_ref(out_frame, in_frame)) < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            }

            ret = avcodec_send_frame(out_codec_ctx, out_frame);
            av_frame_unref(out_frame);
            av_frame_unref(in_frame);
            if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }

            while (ret >= 0) {
                AVPacket *enc_pkt = av_packet_alloc();
                ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_packet_free(&enc_pkt);
                    break;
                } else if (ret < 0) {
                    av_packet_free(&enc_pkt);
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }

                enc_pkt->stream_index = out_stream->index;
                av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

                ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
                av_packet_free(&enc_pkt);
                if (ret < 0) {
                    validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                    goto cleanup;
                }
            }
        }
    }

    ret = avcodec_send_packet(in_codec_ctx, nullptr);
    if (ret < 0 && ret != AVERROR_EOF) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    while (true) {
        ret = avcodec_receive_frame(in_codec_ctx, in_frame);
        if (ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        if (swr_ctx) {
            out_frame->sample_rate = out_codec_ctx->sample_rate;
            out_frame->ch_layout = out_codec_ctx->ch_layout;
            out_frame->format = out_codec_ctx->sample_fmt;
            out_frame->nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, in_codec_ctx->sample_rate) + in_frame->nb_samples,
                                                   out_codec_ctx->sample_rate, in_codec_ctx->sample_rate, AV_ROUND_UP);
            if ((ret = av_frame_get_buffer(out_frame, 0)) < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
            if ((ret = swr_convert(swr_ctx, out_frame->data, out_frame->nb_samples, (const uint8_t**)in_frame->data, in_frame->nb_samples)) < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
        } else {
            if ((ret = av_frame_ref(out_frame, in_frame)) < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
        }

        ret = avcodec_send_frame(out_codec_ctx, out_frame);
        av_frame_unref(out_frame);
        av_frame_unref(in_frame);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        while (ret >= 0) {
            AVPacket *enc_pkt = av_packet_alloc();
            ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                av_packet_free(&enc_pkt);
                break;
            } else if (ret < 0) {
                av_packet_free(&enc_pkt);
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }

            enc_pkt->stream_index = out_stream->index;
            av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

            ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
            av_packet_free(&enc_pkt);
            if (ret < 0) {
                validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
                goto cleanup;
            }
        }
    }

    ret = avcodec_send_frame(out_codec_ctx, nullptr);
    if (ret < 0 && ret != AVERROR_EOF) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
        goto cleanup;
    }

    while (true) {
        AVPacket *enc_pkt = av_packet_alloc();
        ret = avcodec_receive_packet(out_codec_ctx, enc_pkt);
        if (ret == AVERROR_EOF) {
            av_packet_free(&enc_pkt);
            break;
        } else if (ret < 0) {
            av_packet_free(&enc_pkt);
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }

        enc_pkt->stream_index = out_stream->index;
        av_packet_rescale_ts(enc_pkt, in_codec_ctx->time_base, out_stream->time_base);

        ret = av_interleaved_write_frame(out_format_ctx, enc_pkt);
        av_packet_free(&enc_pkt);
        if (ret < 0) {
            validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
            goto cleanup;
        }
    }

    if ((ret = av_write_trailer(out_format_ctx)) < 0) {
        validateclickzWithTimer(hwnd, L"Error Loading File!", RGB(255, 0, 0), 3000);
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

    return ret;
}

   