package com.we.ccamera.presenter;

import android.graphics.ImageFormat;

import java.nio.ByteBuffer;

public class CCameraBuffer {

    public ByteBuffer yBuffer, uBuffer, vBuffer;

    public int frameWidth = 0, frameHeight = 0;

    public byte[] buffer;

    public void setBuffer(ByteBuffer yBuffer, ByteBuffer uBuffer, ByteBuffer vBuffer) {
        this.yBuffer = yBuffer;
        this.uBuffer = uBuffer;
        this.vBuffer = vBuffer;
    }

    public void setBuffer(byte[] buffer) {
        this.buffer = buffer;
    }

    public byte[] initBuffer(int imageWidth, int imageHeight, int imageFormat) {
        int byteCount = imageWidth * imageHeight * ImageFormat.getBitsPerPixel(imageFormat) / 8;
        if(buffer == null || byteCount != buffer.length) {
            buffer = new byte[byteCount];
        }
        return buffer;
    }

}
