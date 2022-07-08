package com.we.ccamera.presenter;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;
import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * 音频录制器
 */
public class FFAudioRecorder {

    private static final String TAG = "FFAudioRecorder";

    private static final int SAMPLE_RATE = 44100;

    private ExecutorService mExecutor = Executors.newCachedThreadPool();

    private AudioRecord mAudioRecord;
    private int mBufferSize;
    private int mSampleRate = SAMPLE_RATE;
    private int mSampleFormat = AudioFormat.ENCODING_PCM_16BIT;
    private int mChannels = 1;

    private OnRecordCallback mRecordCallback;

    private boolean mIsRecording = false;

    public FFAudioRecorder() {
        int channelLayout = (mChannels == 1) ? AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_OUT_STEREO;
        int bufferSize = AudioRecord.getMinBufferSize(mSampleRate, channelLayout, mSampleFormat);
        Log.e(TAG, " Audio record info: "+bufferSize);
    }

    /**
     * 开始录制
     * @return
     */
    public boolean start() {
        try {
            int channelLayout = (mChannels == 1) ? AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_OUT_STEREO;
            mBufferSize = getBufferSize(channelLayout, mSampleFormat);

            mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, mSampleRate,
                    channelLayout, mSampleFormat, mBufferSize);
        } catch (Exception e) {
            Log.e(TAG, "AudioRecord allocator exception: " + e.getLocalizedMessage());
            return false;
        }

        if (mAudioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
            Log.e(TAG, "AudioRecord is uninitialized!");
            return false;
        }
        mIsRecording = true;
        mExecutor.execute(this::record);
        return true;
    }

    /**
     * 录制方法
     */

    private byte[] audioPcmData = null;

    private void record() {
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
        if (mAudioRecord == null || mAudioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
            return;
        }

        ByteBuffer audioBuffer = ByteBuffer.allocate(mBufferSize);

        mAudioRecord.startRecording();

        mRecordCallback.onRecordStart();

        int readResult;
        while (mIsRecording)
        {
            readResult = mAudioRecord.read(audioBuffer.array(), 0, mBufferSize);
            if (readResult > 0 && mRecordCallback != null)
            {
                if(audioPcmData == null) {
                    audioPcmData = new byte[readResult];
                }
                audioBuffer.position(0);
                audioBuffer.limit(readResult);
                audioBuffer.get(audioPcmData, 0, readResult);
                mRecordCallback.onRecordSample(audioPcmData);
            }
        }

        release();

        mRecordCallback.onRecordFinish();
    }

    /**
     * 停止录制
     */
    public void stop() {
        mIsRecording = false;
    }

    /**
     * 释放资源
     */
    private void release() {
        if (mAudioRecord != null) {
            mAudioRecord.stop();
            mAudioRecord.release();
            mAudioRecord = null;
        }
    }

    /**
     * 获取缓冲区大小
     * @param channelLayout
     * @param pcmFormat
     * @return
     */
    private int getBufferSize(int channelLayout, int pcmFormat) {

        int bufferSize = 1024;

        switch (channelLayout) {
            case AudioFormat.CHANNEL_IN_MONO: {
                bufferSize *= 1;
                break;
            }

            case AudioFormat.CHANNEL_IN_STEREO: {
                bufferSize *= 2;
                break;
            }
        }

        switch (pcmFormat) {
            case AudioFormat.ENCODING_PCM_8BIT: {
                bufferSize *= 1;
                break;
            }

            case AudioFormat.ENCODING_PCM_16BIT: {
                bufferSize *= 2;
                break;
            }
        }

        //bufferSize *= 2;

        return bufferSize;
    }

    /**
     * 设置采样率
     * @param sampleRate
     */
    public void setSampleRate(int sampleRate) {
        mSampleRate = sampleRate;
    }

    /**
     * 获取采样率
     * @return
     */
    public int getSampleRate() {
        return mSampleRate;
    }

    /**
     * 设置采样格式
     * @param format
     */
    public void setSampleFormat(int format) {
        mSampleFormat = format;
    }

    /**
     * 获取采样格式
     * @return
     */
    public int getSampleFormat() {
        return mSampleFormat;
    }

    /**
     * 设置声道数
     * @param channels
     */
    public void setChannels(int channels) {
        mChannels = channels;
    }

    /**
     * 获取声道数
     * @return
     */
    public int getChannels() {
        return mChannels;
    }

    /**
     * 设置录制回调
     */
    public interface OnRecordCallback {
        // 录制开始
        void onRecordStart();

        // 录制PCM采样回调
        void onRecordSample(byte[] data);

        // 录制结束
        void onRecordFinish();
        
    }

    /**
     * 设置录制回调
     * @param callback
     */
    public void setOnRecordCallback(OnRecordCallback callback) {
        mRecordCallback = callback;
    }
}
