package com.we.ccamera;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import com.we.ccamera.presenter.CCRecordPresenter;
import com.we.ccamera.renderer.CameraRenderer;


public class MainActivity extends AppCompatActivity {

    private CCRecordPresenter presenter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, 1001);
            return;
        }

        presenter = new CCRecordPresenter(this);
        initView();
    }

    private GLSurfaceView glSurfaceView;
    private void initView() {
        glSurfaceView = findViewById(R.id.gl_surface_view);
        glSurfaceView.setEGLContextClientVersion(3);
        CameraRenderer cameraRenderer = new CameraRenderer();
        cameraRenderer.setOesTextureListener((SurfaceTexture surfaceTexture)->
        {
            presenter.openCamera(surfaceTexture);
        });
        glSurfaceView.setRenderer(cameraRenderer);
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        findViewById(R.id.start_button).setOnClickListener(this::onStartRecord);
        findViewById(R.id.stop_button).setOnClickListener(this::onStopRecord);
    }

    public void onFrameAvailable() {
        glSurfaceView.requestRender();
    }

    private void onStartRecord(View clickView) {
        presenter.startRecord();
    }

    private void onStopRecord(View clickView) {
        presenter.stopRecord();
    }

    @Override
    protected void onResume() {
        super.onResume();
        presenter.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        presenter.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        presenter.onDestroy();
    }
}