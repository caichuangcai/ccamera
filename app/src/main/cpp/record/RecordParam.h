//
// Created by Administrator on 2022/4/26.
//

#ifndef CCAMERA_RECORDPARAM_H
#define CCAMERA_RECORDPARAM_H


class RecordParam {
public:
    int videoWidth             = 0;    // 分辨率宽度
    int videoHeight            = 0;   // 分辨率高度
    int videoFrameRate         = 25;     // 视频帧率
    int pixelFormat            = 4;      // 像素格式
    int videoQuality           = 23;     // 质量参数
    const char* server         = "rtmp://10.21.252.101:1935/hls/test"; // "rtmp://10.21.252.101:1935/live/test"; "/storage/emulated/0/1/aaa.flv"

    int sampleRate    = 44100;         // 音频采样率
    int channels      = 1;      // 声道数
};

#endif //CCAMERA_RECORDPARAM_H
