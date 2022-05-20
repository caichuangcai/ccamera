//
// Created by Administrator on 2022/1/21.
//

#ifndef CCAMERA_AUDIOENCODER_H
#define CCAMERA_AUDIOENCODER_H

#include "MediaHeader.h"
#include <map>
#include <string>

class AudioEncoder {
public:
    AudioEncoder();

    virtual ~AudioEncoder();

    void setFormatContext(AVFormatContext *_format_ctx);

    void setAudioParams(int bitrate, int sampleRate, int channels);

    void createEncoder();

    void openEncoder(std::map<std::string, std::string> mEncodeOptions);

    int encodeFrame(AVFrame *frame);

    AVCodecContext* getContext();

    void closeEncoder();

    void reset();

private:

    AVFormatContext *pFormatCtx;    // 复用上下文
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    AVPacket *vPacket;

    AVStream *pStream;

};


#endif //CCAMERA_AUDIOENCODER_H
