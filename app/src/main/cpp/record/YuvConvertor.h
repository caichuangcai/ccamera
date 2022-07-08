#ifndef YUVCONVERTOR_H
#define YUVCONVERTOR_H

#include "AVMediaData.h"
#include "YuvData.h"
#include <stdio.h>

/**
 * YUV转换器
 */
class YuvConvertor {

public:
    YuvConvertor();

    virtual ~YuvConvertor();

    // 设置输入参数
    void setInputParams(int width, int height, int pixelFormat);

    // 设置裁剪区域
    void setCrop(int x, int y, int width, int height);

    // 设置旋转角度
    void setRotate(int degree);

    // 准备yuv转换器
    int prepare();

    // 转换数据
    int convert(AVMediaData *mediaData);

private:
    // 重置所有参数
    void reset();

    // 释放所有资源
    void release();

    // 填充媒体数据
    void fillMediaData(AVMediaData *model, YuvData *src, int srcW, int srcH);

private:

    int mWidth;             // 源宽度
    int mHeight;            // 源高度
    int mPixelFormat;       // 图像格式
    bool mNeedConvert;    // 是否允许转换

    int mCropX;
    int mCropY;
    int mCropWidth;
    int mCropHeight;
    libyuv::RotationMode mRotationMode;

    YuvData *pCropData;

    uint8_t count;

};


#endif //YUVCONVERTOR_H
