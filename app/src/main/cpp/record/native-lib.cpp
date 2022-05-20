#include <jni.h>
#include <string.h>
#include "MediaRecorder.h"
#include "MediaHeader.h"

extern "C" {

    MediaRecorder *mediaRecorder;
    uint8_t *yuvData; // yuv缓冲区

    JNIEXPORT void JNICALL Java_com_we_ccamera_presenter_CCMediaRecorder_nativeInit(
            JNIEnv* env,
            jobject /* this */, jint width, jint height) {

        mediaRecorder = new MediaRecorder();
        mediaRecorder->setVideoInfo(width, height);
        mediaRecorder->prepare();
    }

    JNIEXPORT void JNICALL Java_com_we_ccamera_presenter_CCMediaRecorder_nativeSetVideoParams(
            JNIEnv* env,
            jobject /* this */,
            jint width, jint height, jint frameRate, jint pixelFormat, jlong maxBitRate, jint quality) {

        if(mediaRecorder == nullptr) {
            LOGE("setVideoParams error,  MediaRecorder is null");
            return ;
        }

    }

    JNIEXPORT void JNICALL Java_com_we_ccamera_presenter_CCMediaRecorder_nativeSetAudioParams(
            JNIEnv* env,
            jobject /* this */,
            jint sampleRate, jint sampleFormat, jint channels) {

    }

    JNIEXPORT void JNICALL Java_com_we_ccamera_presenter_CCMediaRecorder_nativeStartRecord(
            JNIEnv* env,
            jobject /* this */) {

        if(mediaRecorder == nullptr) {
            LOGE("startRecord error,  MediaRecorder is null");
            return ;
        }

        mediaRecorder->startRecord();
    }

    JNIEXPORT void JNICALL Java_com_we_ccamera_presenter_CCMediaRecorder_nativeStopRecord(
            JNIEnv* env,
            jobject /* this */) {

        if(mediaRecorder == nullptr) {
            LOGE("stopRecord error,  MediaRecorder is null");
            return ;
        }

        mediaRecorder->stopRecord();
    }

    JNIEXPORT void JNICALL Java_com_we_ccamera_presenter_CCMediaRecorder_nativeSendVideoFrame(
            JNIEnv* env,
            jobject /* this */,
            jbyteArray jdata, jint data_length, jint width, jint height) {

        if(mediaRecorder == nullptr) {
            LOGE("sendVideoFrame error,  MediaRecorder is null");
            return ;
        }

        if(!mediaRecorder->isRecoding()) {
            LOGE("MediaRecorder has stopped.");
            return ;
        }


        uint8_t *yuvData = (uint8_t *) malloc((size_t) data_length);

        if (yuvData == nullptr) {
            LOGE("Could not allocate yuv data");
            return ;
        }

        jbyte *data = env->GetByteArrayElements(jdata, nullptr);
        memcpy(yuvData, data, (size_t) data_length);
        env->ReleaseByteArrayElements(jdata, data, 0);

        auto mediaData = new AVMediaData();
        mediaData->setVideo(yuvData, data_length, width, height, 1);
        mediaData->setPts(getCurrentTimeMs());

        mediaRecorder->recordFrame(mediaData);
    }

    JNIEXPORT void JNICALL Java_com_we_ccamera_presenter_CCMediaRecorder_nativeSendAudioFrame(
            JNIEnv* env,
            jobject /* this */,
            jbyteArray _data, jint len) {

        if(mediaRecorder == nullptr) {
            LOGE("sendVideoFrame error,  MediaRecorder is null");
            return ;
        }

        if(!mediaRecorder->isRecoding()) {
            LOGE("MediaRecorder has stopped.");
            return ;
        }

        uint8_t *pcmData = (uint8_t *) malloc((size_t) len);
        if (pcmData == nullptr) {
            LOGE("Could not allocate memory");
            return ;
        }
        jbyte *data = env->GetByteArrayElements(_data, nullptr);
        memcpy(pcmData, data, (size_t) len);
        env->ReleaseByteArrayElements(_data, data, 0);

        auto mediaData = new AVMediaData();
        mediaData->setAudio(pcmData, len);
        mediaData->setPts(getCurrentTimeMs());
        mediaRecorder->recordFrame(mediaData);
    }

}