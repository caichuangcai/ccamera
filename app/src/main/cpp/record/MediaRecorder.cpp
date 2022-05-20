//
// Created by Administrator on 2022/4/25.
//

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
        LOGE("MediaMuxer is working");
        return ;
    }

    LOGE("MediaRecorder prepare   av_version_info: %s", av_version_info());

    mFrameQueue = new SafetyQueue<AVMediaData*>();

    mYuvConvertor = new YuvConvertor();
    mYuvConvertor->setInputParams(recordParam->videoHeight, recordParam->videoWidth, recordParam->pixelFormat);
    mYuvConvertor->setCrop(0, 0, 0, 0);
    mYuvConvertor->setScale(0, 0);
    mYuvConvertor->setRotate(libyuv::kRotate90);
    mYuvConvertor->setMirror(false);

    int ret = mYuvConvertor->prepare();
    LOGE("MediaRecorder yuvConvertor.prepare() ret: %d", ret);
    if (ret < 0) {
        delete mYuvConvertor;
        mYuvConvertor = nullptr;
    }

    mediaMuxer->addEncodeOptions("preset", "veryfast");
    //mediaMuxer->addEncodeOptions("tune", "zerolatency");
    //mediaMuxer->addEncodeOptions("profile", "baseline");
    //mediaMuxer->setQuality(recordParam->videoQuality > 0 ? recordParam->videoQuality : 29);
    mediaMuxer->setOutputPath(recordParam->server);

    ret = mediaMuxer->prepare();
    if (ret < 0) {
        //release();
        LOGE("Record is not ready...");
        return ;
    }
    LOGE("Record is ready...");
}

// 开始录制
void MediaRecorder::startRecord() {
    mMutex.lock();
    mAbortRequest = false;
    mStartRequest = true;
    mExit = false;
    mCondition.signal();
    mMutex.unlock();

    LOGE("MediaRecorder  startRecord  ====  %s", mRecordThread==nullptr?"mRecordThread is null":"mRecordThread is not null");

    if(mRecordThread != nullptr) {
        mRecordThread = nullptr;
    }

    if (mRecordThread == nullptr) {
        mRecordThread = new Thread(this);
        mRecordThread->start();
        //mRecordThread->detach();
    }
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
        //mRecordThread->join();
        delete mRecordThread;
        mRecordThread = nullptr;
    }

    delete mediaMuxer;
    mediaMuxer = NULL;
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

    if(mFrameQueue != nullptr) {
        mFrameQueue->push(data);
    } else {
        delete data;
    }
    return 0;
}

void MediaRecorder::run() {

    mExit = false;

    /* select the stream to encode */
    int encode_video = 1, encode_audio = 1;

    while (!mStartRequest) {
        if (mAbortRequest) { // 停止请求则直接退出
            break;
        } else { // 睡眠10毫秒继续
            av_usleep(10 * 1000);
        }
    }

    if (!mAbortRequest && mStartRequest)
    {
        while (!mAbortRequest && mStartRequest)
        {
            if (!mFrameQueue->empty())
            {
                auto data = mFrameQueue->pop();
                if (!data) {
                    continue;
                }

                //LOGE("MediaRecorder  run   encode_video: %d, encode_audio: %d", encode_video, encode_audio);

                if (encode_video &&
                    (!encode_audio || mediaMuxer->encodeVideoNow()))
                {
                    if(data->getType() == MediaAudio) {
                        delete data;
                        continue;
                    }
                    if (mYuvConvertor->convert(data) < 0) {
                        LOGE("Failed to convert video data to yuv420");
                        delete data;
                        continue;
                    }
                    encode_video = !mediaMuxer->encodeMediaData(data);
                }
                else {
                    //LOGE("MediaRecorder  encode audio   %s", data->getType()==MediaVideo?"MediaVideo":"MediaAudio");
                    if(data->getType() == MediaVideo) {
                        delete data;
                        continue;
                    }
                    encode_audio = !mediaMuxer->encodeMediaData(data);
                }

                // 释放资源
                delete data;
            }
        }

        LOGE("MediaRecorder  run  after stop record, mFrameQueue->size(): %d", mFrameQueue->size());

        mediaMuxer->stop();

    }

    mediaMuxer->release();

    mCondition.signal();

    LOGE("MediaRecorder  loop  run   end ===== ");
}


void MediaRecorder::release() {
    stopRecord();
    mMutex.lock();
    while (!mExit) {
        mCondition.wait(mMutex);
    }
    mMutex.unlock();

    if (mFrameQueue != nullptr) {
        delete mFrameQueue;
        mFrameQueue = nullptr;
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