//
// Created by Administrator on 2022/1/21.
//

#ifndef CCAMERA_VIDEOENCODER_H
#define CCAMERA_VIDEOENCODER_H

#include "MediaHeader.h"
#include <map>
#include <string>

class VideoEncoder {
public:

    VideoEncoder();

    virtual ~VideoEncoder();

    void setFormatContext(AVFormatContext * _format_ctx);

    void createEncoder();

    void openEncoder(std::map<std::string, std::string> mEncodeOptions);

    // 设置视频参数
    void setVideoParams(int width, int height, AVPixelFormat pixelFormat, int frameRate,
                        int maxBitRate, std::map<std::string, std::string> metadata);

    int encodeFrame(AVFrame *frame);

    void closeEncoder();

    AVCodecContext* getCodecContext();

    AVRational getStreamTimeBase();

private:

    AVFormatContext *pFormatCtx;    // 复用上下文
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVStream *pStream;

    AVPacket *vPacket;

};


#endif //CCAMERA_VIDEOENCODER_H
