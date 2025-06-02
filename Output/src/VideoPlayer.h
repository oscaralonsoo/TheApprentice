#pragma once
#include <string>
#include <SDL2/SDL.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();

    bool Load(const std::string& path, SDL_Renderer* renderer);
    void Play();                 // Inicializa la reproducción
    bool Update();               // Reproduce un frame, devuelve false si terminó
    void CleanUp();

private:
    AVFormatContext* pFormatCtx = nullptr;
    AVCodecContext* pCodecCtx = nullptr;
    AVCodecParameters* pCodecParams = nullptr;
    const AVCodec* pCodec = nullptr;
    AVFrame* pFrame = nullptr;
    AVFrame* pFrameRGB = nullptr;
    struct SwsContext* sws_ctx = nullptr;
    int videoStream = -1;

    SDL_Texture* texture = nullptr;
    SDL_Renderer* renderer = nullptr;

    int width = 0;
    int height = 0;

    bool isPlaying = false;
};
