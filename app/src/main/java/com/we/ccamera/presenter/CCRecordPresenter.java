package com.we.ccamera.presenter;

import android.graphics.SurfaceTexture;
import android.media.AudioFormat;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.we.ccamera.MainActivity;
import com.we.ccamera.cameracontrol.CameraController;
import com.we.ccamera.cameracontrol.ICameraController;

import java.io.File;
import java.io.FileOutputStream;


public class CCRecordPresenter implements OnSurfaceTextureListener,
        OnFrameAvailableListener, PreviewCallback, FFAudioRecorder.OnRecordCallback  {

    private final ICameraController cameraController;

    private CCMediaRecorder mediaRecorder;
    //private CCAudioRecorder audioRecorder;

    private FFAudioRecorder mFFAudioRecorder;


    private MainActivity context;

    private Handler mHandler;

    private volatile boolean mIsRecording = false;

    public CCRecordPresenter(MainActivity context) {
        this.context = context;
        cameraController = new CameraController(context);
        mediaRecorder = new CCMediaRecorder();

        mFFAudioRecorder = new FFAudioRecorder();
        mFFAudioRecorder.setOnRecordCallback(this);
        mFFAudioRecorder.setSampleFormat(AudioFormat.ENCODING_PCM_16BIT);

        mHandler = new Handler(Looper.getMainLooper());

        cameraController.setOnFrameAvailableListener(this);
        cameraController.setOnPreviewCallback(this);
        cameraController.setOnSurfaceTextureListener(this);

        mediaRecorder.init(720, 1280);
    }

    public void openCamera(SurfaceTexture surfaceTexture) {
        cameraController.openCamera(surfaceTexture);
    }

    @Override
    public void onRecordSample(byte[] data) {
        if(!mIsRecording) {
            return ;
        }
        //Log.e("CCamera", "onRecordSample === data.len: "+data.length);
        mediaRecorder.recordAudioFrame(data, data.length);
    }

    @Override
    public void onRecordFinish() {
        Log.e("CCamera", " == onRecordFinish");
    }

    public void startRecord() {
        mIsRecording = true;

        mFFAudioRecorder.start();
        mediaRecorder.startRecord();
    }

    public void stopRecord() {
        mIsRecording = false;
        mFFAudioRecorder.stop();
        mediaRecorder.stopRecord();
    }

    @Override
    public void onRecordStart() {
        Log.e("CCamera", " == onRecordStart");
    }

    @Override
    public void onFrameAvailable() {
        context.onFrameAvailable();
    }

    @Override
    public void onSurfaceTexturePrepared(int videoWidth, int videoHeight) {

    }

    @Override
    public void onPreviewFrame(CCameraBuffer cCameraBuffer) {
        if(!mIsRecording || cCameraBuffer.buffer == null) {
            return ;
        }
        mediaRecorder.recordVideoFrame(cCameraBuffer.buffer, cCameraBuffer.buffer.length, cCameraBuffer.frameWidth, cCameraBuffer.frameHeight);
    }

    boolean flag = false;
    private void writeBuffer(byte[] data) {
        if(flag) {
            return ;
        }
        flag = true;
        try {
            File file = new File("/storage/emulated/0/1/ccc.yuv");
            FileOutputStream outputStream = new FileOutputStream(file);
            outputStream.write(data);
            outputStream.flush();
            outputStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void onResume() {

    }

    public void onPause() {

    }

    public void onDestroy() {

    }

}