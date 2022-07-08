package com.we.ccamera.presenter;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;

public class CCMediaRecorder {

    private final String TAG = CCMediaRecorder.class.getName();

    static {
        System.loadLibrary("CCMedia");
    }

    private Handler mHandler = null;

    public CCMediaRecorder() {
        mHandler = new Handler(Looper.getMainLooper());
    }

    public void init(int width, int height) {
        mHandler.post(()->nativeInit(width, height));
    }

    public void setVideoParam(int width, int height, int frameRate, int pixelFormat, long maxBitRate, int quality) {
        mHandler.post(()->nativeSetVideoParams(width, height, frameRate, pixelFormat, maxBitRate, quality));
    }

    public void setAudioParams(int sampleRate, int sampleFormat, int channels) {
        mHandler.post(()->nativeSetAudioParams(sampleRate, sampleFormat, channels));
    }

    public void recordVideoFrame(byte[] data, int length, int recordWidth, int recordHeight) {
        mHandler.post(()->nativeSendVideoFrame(data, length, recordWidth, recordHeight));
    }

    public void recordAudioFrame(byte[] data, int len) {
        mHandler.post(()->nativeSendAudioFrame(data, len));
    }

    public void startRecord() {
        mHandler.post(CCMediaRecorder::nativeStartRecord);
    }

    public void stopRecord() {
        mHandler.post(CCMediaRecorder::nativeStopRecord);
    }

    public static native void nativeInit(int width, int height);

    public static native void nativeSetVideoParams(int width, int height, int frameRate, int pixelFormat, long maxBitRate, int quality);

    public static native void nativeSetAudioParams(int sampleRate, int sampleFormat, int channels);

    public static native void nativeSendVideoFrame(byte[] data, int length, int width, int height);

    public static native void nativeSendAudioFrame(byte[] data, int len);

    public static native void nativeStartRecord();

    public static native void nativeStopRecord();

}