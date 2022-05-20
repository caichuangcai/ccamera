package com.we.ccamera.cameracontrol;

import android.graphics.SurfaceTexture;

import com.we.ccamera.presenter.OnFrameAvailableListener;
import com.we.ccamera.presenter.OnSurfaceTextureListener;
import com.we.ccamera.presenter.PreviewCallback;

public interface ICameraController {

    void openCamera(SurfaceTexture surfaceTexture);

    void closeCamera();

    void setFront(boolean front);

    boolean isFront();

    int getPreviewWidth();

    int getPreviewHeight();

    void setOnSurfaceTextureListener(OnSurfaceTextureListener listener);

    void setOnPreviewCallback(PreviewCallback callback);

    void setOnFrameAvailableListener(OnFrameAvailableListener listener);

}
