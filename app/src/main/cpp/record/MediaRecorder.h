//
// Created by Administrator on 2022/4/25.
//

#ifndef CCAMERA_MEDIARECORDER_H
#define CCAMERA_MEDIARECORDER_H

#include <Thread.h>
#include <Mutex.h>
#include "YuvConvertor.h"
#include <libyuv.h>
#include "MediaMuxer.h"
#include "MediaHeader.h"
#include "RecordParam.h"
#include "AVMediaData.h"
#include "SafetyQueue.h"

class MediaRecorder : public Runnable {

public:
    MediaRecorder();

    virtual ~MediaRecorder();

    void prepare();

    void setVideoInfo(int width, int height);

    int recordFrame(AVMediaData *data);

    void startRecord();

    void stopRecord();

    void run();

    bool isRecoding();

    void release();

private:
    Mutex mMutex;
    Condition mCondition;
    Thread *mRecordThread = nullptr;

    // 视频数据队列
    SafetyQueue<AVMediaData*> *mVideoQueue = nullptr;
    // 音频数据队列
    SafetyQueue<AVMediaData*> *mAudioQueue = nullptr;

    bool mAbortRequest = false; // 停止请求
    bool mStartRequest = false; // 开始录制请求
    bool mExit = false;         // 完成退出标志

    YuvConvertor *mYuvConvertor = nullptr;    // Yuv转换器

    MediaMuxer* mediaMuxer = nullptr;    // 音频、视频流

    RecordParam* recordParam = nullptr;

};


#endif //CCAMERA_MEDIARECORDER_H
