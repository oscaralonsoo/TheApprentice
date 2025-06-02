#include "VideoPlayer.h"
#include <iostream>

VideoPlayer::VideoPlayer() {
    avformat_network_init();
}

VideoPlayer::~VideoPlayer() {
    CleanUp();
}

bool VideoPlayer::Load(const std::string& path, SDL_Renderer* rend) {
    renderer = rend;

    if (avformat_open_input(&pFormatCtx, path.c_str(), nullptr, nullptr) != 0) {
        return false;
    }

    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        return false;
    }

    for (unsigned int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        return false;
    }

    pCodecParams = pFormatCtx->streams[videoStream]->codecpar;
    pCodec = avcodec_find_decoder(pCodecParams->codec_id);
    if (!pCodec) {
        return false;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        return false;
    }

    if (avcodec_parameters_to_context(pCodecCtx, pCodecParams) < 0) {
        return false;
    }

    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        return false;
    }

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    if (!pFrame || !pFrameRGB) {
        return false;
    }

    width = pCodecCtx->width;
    height = pCodecCtx->height;

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer,
        AV_PIX_FMT_RGB24, width, height, 1);

    sws_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt,
        width, height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!texture) {
        std::cerr << "Error: No se pudo crear la textura SDL." << std::endl;
        return false;
    }

    isPlaying = true;
    return true;
}

void VideoPlayer::Play() {
}

bool VideoPlayer::Update() {
    if (!isPlaying) return false;

    AVPacket packet;
    int response;

    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
            response = avcodec_send_packet(pCodecCtx, &packet);
            if (response < 0) {
                av_packet_unref(&packet);
                return false;
            }

            response = avcodec_receive_frame(pCodecCtx, pFrame);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                av_packet_unref(&packet);
                continue;
            }
            else if (response < 0) {
                av_packet_unref(&packet);
                return false;
            }

            sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, height,
                pFrameRGB->data, pFrameRGB->linesize);

            SDL_UpdateTexture(texture, nullptr, pFrameRGB->data[0], pFrameRGB->linesize[0]);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);

            av_packet_unref(&packet);

            SDL_Delay(1000 / 30);  // Aproximadamente 30 FPS
            return true;  // Frame renderizado, seguimos reproduciendo
        }
        av_packet_unref(&packet);
    }

    // Si llegamos aquí, se terminó el video
    isPlaying = false;
    return false;
}

void VideoPlayer::CleanUp() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (pFrameRGB) {
        av_free(pFrameRGB->data[0]);
        av_frame_free(&pFrameRGB);
    }
    if (pFrame) av_frame_free(&pFrame);
    if (pCodecCtx) avcodec_free_context(&pCodecCtx);
    if (pFormatCtx) avformat_close_input(&pFormatCtx);
    if (sws_ctx) sws_freeContext(sws_ctx);
}
