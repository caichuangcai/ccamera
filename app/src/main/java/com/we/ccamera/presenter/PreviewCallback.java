package com.we.ccamera.presenter;


/**
 * PreviewCallback in camera2   视频颜色空间：YUV_420_8888
 *
 * */
public interface PreviewCallback {

    void onPreviewFrame(CCameraBuffer buffer);

}