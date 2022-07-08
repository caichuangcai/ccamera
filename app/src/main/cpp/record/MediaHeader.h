#ifndef CCAMERA_MEDIAHEADER_H
#define CCAMERA_MEDIAHEADER_H

#include <android/log.h>
#define JNI_TAG "CCamera"
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR,   JNI_TAG, format, ##__VA_ARGS__)
#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO,    JNI_TAG, format, ##__VA_ARGS__)

#include <cstdint>
#include <libyuv.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avstring.h>
#include <libavutil/eval.h>
#include <libavutil/display.h>
#include <libavutil/pixfmt.h>

#ifdef __cplusplus
};
#endif

#ifndef AUDIO_MIN_BUFFER_SIZE
#define AUDIO_MIN_BUFFER_SIZE 512
#endif

#ifndef AUDIO_MAX_CALLBACKS_PER_SEC
#define AUDIO_MAX_CALLBACKS_PER_SEC 30
#endif

#ifndef AUDIO_BIT_RATE
#define AUDIO_BIT_RATE 128000
#endif

// 1280 * 720
#ifndef VIDEO_BIT_RATE
#define VIDEO_BIT_RATE 6693560
#endif

// 576 * 1024
#ifndef VIDEO_BIT_RATE_LOW
#define VIDEO_BIT_RATE_LOW 3921332
#endif

// 获取当前时钟(ms)
inline uint64_t getCurrentTimeMs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t us = (uint64_t) (tv.tv_sec) * 1000 * 1000 + (uint64_t) (tv.tv_usec);
    return us / 1000;
}

#endif //CCAMERA_MEDIAHEADER_H
