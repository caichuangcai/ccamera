
#ifndef CCAMERA_MEDIAMUXER_H
#define CCAMERA_MEDIAMUXER_H

#include <jni.h>
#include <map>
#include <cstdint>
#include "AudioEncoder.h"
#include "VideoEncoder.h"
#include "MediaHeader.h"
#include "AVMediaData.h"
#include "Resampler.h"
#include <sstream>
#include <string.h>

class MediaMuxer {

public:

    MediaMuxer();

    virtual ~MediaMuxer();

    void init();

    void setOutputPath(const char* path);

    int openVideoEncoder();

    int openAudioEncoder();

    void startRecord();

    void stopRecord();

    // 设置质量系数
    void setQuality(int quality);

    int prepare();

    int encodeMediaData(AVMediaData *mediaData);

    //int encodeMediaData(AVMediaData *mediaData, int *gotFrame);

    int fillImage(AVMediaData *data);

    int fillSample(AVMediaData *data);

    void flush();

    int stop();

    void release();

    // 添加编码参数
    void addEncodeOptions(std::string key, std::string value);

    bool encodeVideoNow();

private:

    std::map<std::string, std::string> mEncodeOptions;  // 编码参数
    std::map<std::string, std::string> mVideoMetadata;  // 视频meta数据

    char* outputPath;
    int videoWidth             = 720;    // 分辨率宽度
    int videoHeight            = 1280;   // 分辨率高度
    int videoFrameRate         = 20;     // 视频帧率
    AVPixelFormat mPixelFormat = AV_PIX_FMT_YUV420P;     // 像素格式

    int sampleRate    = 44100;         // 音频采样率
    int channels      = 1;      // 声道数
    AVSampleFormat mSampleFormat = AV_SAMPLE_FMT_S16;   // 采样格式

    uint64_t video_next_pts = 0;    // 视频帧数

    AVFrame *mImageFrame;           // 视频缓冲帧
    uint8_t *mImageBuffer;          // 视频缓冲区

    AVFormatContext *pFormatCtx = nullptr;    // 上下文
    VideoEncoder *videoEncoder = nullptr;
    AVCodecContext *videoCodecCtx = nullptr;  // 视频编码上下文

    AudioEncoder *audioEncoder = nullptr;
    AVCodecContext *audioCodecCtx = nullptr;  // 音频编码上下文
    Resampler *mResampler = nullptr;

};

#endif //CCAMERA_MEDIAMUXER_H