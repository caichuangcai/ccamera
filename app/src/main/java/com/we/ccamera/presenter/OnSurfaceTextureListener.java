package com.we.ccamera.presenter;

import android.graphics.SurfaceTexture;

import androidx.annotation.NonNull;

public interface OnSurfaceTextureListener {

    void onSurfaceTexturePrepared(int videoWidth, int videoHeight);

}