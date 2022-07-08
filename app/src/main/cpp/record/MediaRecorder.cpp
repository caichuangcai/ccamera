#include "MediaRecorder.h"

MediaRecorder::MediaRecorder() : mAbortRequest(true), mExit(true) {
    recordParam = new RecordParam();

    mediaMuxer = new MediaMuxer();
    mediaMuxer->init();
}

void MediaRecorder::setVideoInfo(int width, int height) {
    recordParam->videoWidth = width;
    recordParam->videoHeight = height;
    LOGE("MediaRecorder setVideoInfo  width: %d, height: %d", width, height);
}

void MediaRecorder::prepare() {

    if(mediaMuxer == nullptr) {
        LOGE("MediaRecorder prepare fail, mediaMuxer is null");
        return ;
    }

    LOGE("FFmpeg  av_version_info: %s", av_version_info());

    mVideoQueue = new SafetyQueue<AVMediaData*>();

    mAudioQueue = new SafetyQueue<AVMediaData*>();

    mYuvConvertor = new YuvConvertor();
    mYuvConvertor->setInputParams(recordParam->videoHeight, recordParam->videoWidth, recordParam->pixelFormat);
    mYuvConvertor->setCrop(0, 0, 0, 0);
    mYuvConvertor->setRotate(libyuv::kRotate90);

    int ret = mYuvConvertor->prepare();
    if (ret < 0) {
        LOGE("mYuvConvertor prepare fail,  ret: %d", ret);
        delete mYuvConvertor;
        mYuvConvertor = nullptr;
    }

    mediaMuxer->addEncodeOptions("preset", "veryfast");
    mediaMuxer->setOutputPath(recordParam->server);

    ret = mediaMuxer->prepare();
    if (ret < 0) {
        //release();
        LOGE("MediaRecorder is not ready, mediaMuxer prepare fail, ret: %d", ret);
        return ;
    }
    LOGE("MediaRecorder is ready");
}

// 开始录制
void MediaRecorder::startRecord() {
    mAbortRequest = false;
    mStartRequest = true;
    mExit = false;

    mRecordThread = new Thread(this);
    mRecordThread->start();
}

// 停止录制
void MediaRecorder::stopRecord() {
    mMutex.lock();
    mAbortRequest = true;
    mStartRequest = false;
    mExit = true;
    mCondition.signal();
    mMutex.unlock();

    if (mRecordThread != nullptr) {
        delete mRecordThread;
        mRecordThread = nullptr;
    }

    delete mediaMuxer;
    mediaMuxer = nullptr;
}

// 是否正在录制
bool MediaRecorder::isRecoding() {
    return !mAbortRequest && mStartRequest && !mExit;
}

// 编码数据，放进队列
int MediaRecorder::recordFrame(AVMediaData *data) {
    if (mAbortRequest || mExit) {
        LOGE("Recoder is not recording.");
        delete data;
        return -1;
    }
    if(mVideoQueue->size() <= 30 && data->type == MediaVideo) {
        mVideoQueue->push(data);
    }
    else if(mAudioQueue->size() <= 30 && data->type == MediaAudio) {
        mAudioQueue->push(data);
    }
    else {
        delete data;
    }
    return 1;
}

void MediaRecorder::run() {

    mExit = false;

    while (!mStartRequest) {
        if (mAbortRequest) { // 停止请求则直接退出
            break;
        } else { // 睡眠10毫秒继续
            av_usleep(10 * 1000);
        }
    }

    /**
     * 选择视频还是音频编码
     * */
    int encode_video = 1, encode_audio = 1;

    if (!mAbortRequest && mStartRequest)
    {
        while (!mAbortRequest && mStartRequest)
        {
            // 视频编码
            if (encode_video &&
                (!encode_audio || mediaMuxer->encodeVideoNow()))
            {
                if(mVideoQueue->empty()) {
                    continue;
                }
                auto data = mVideoQueue->pop();
                if (mYuvConvertor->convert(data) < 0) {
                    LOGE("Failed to convert video data to yuv420");
                    delete data;
                    continue;
                }
                encode_video = !mediaMuxer->encodeMediaData(data);
            }
            // 音频编码
            else {
                if(mAudioQueue->empty()) {
                    continue;
                }
                auto data = mAudioQueue->pop();
                encode_audio = !mediaMuxer->encodeMediaData(data);
            }
        }

        LOGE("MediaRecorder  run  after stop record, mAudioQueue.size: %d, mVideoQueue.size: %d", mAudioQueue->size(), mVideoQueue->size());

        mediaMuxer->flush();

        mediaMuxer->stop();
    }

    mediaMuxer->release();

    mCondition.signal();

    LOGE("MediaRecorder  run   end  ===== ");
}

void MediaRecorder::release() {
    stopRecord();
    mMutex.lock();
    while (!mExit) {
        mCondition.wait(mMutex);
    }
    mMutex.unlock();

    if(mVideoQueue != nullptr) {
        delete mVideoQueue;
        mVideoQueue = nullptr;
    }

    if(mAudioQueue != nullptr) {
        delete mAudioQueue;
        mAudioQueue = nullptr;
    }

    if (mRecordThread != nullptr) {
        delete mRecordThread;
        mRecordThread = nullptr;
    }

    if(mediaMuxer != nullptr) {
        delete mediaMuxer;
        mediaMuxer = nullptr;
    }

}

MediaRecorder::~MediaRecorder() {
    release();
    if (recordParam != nullptr) {
        delete recordParam;
        recordParam = nullptr;
    }
}