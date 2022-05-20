//
// Created by CainHuang on 2019/8/24.
//

#include "YuvConvertor.h"

YuvConvertor::YuvConvertor() {
    reset();
}

YuvConvertor::~YuvConvertor() {
    release();
}

/**
 * 重置所有参数
 */
void YuvConvertor::reset() {
    mNeedConvert = false;
    mCropX = 0;
    mCropY = 0;
    mCropWidth = 0;
    mCropHeight = 0;
    mRotationMode = libyuv::kRotate0;
    mScaleWidth = 0;
    mScaleHeight = 0;
    mMirror = false;

    pCropData = nullptr;
    pScaleData = nullptr;
    pMirrorData = nullptr;
}

/**
 * 释放所有资源
 */
void YuvConvertor::release() {
    if (pCropData != nullptr) {
        delete pCropData;
        pCropData = nullptr;
    }

    if (pScaleData != nullptr) {
        delete pScaleData;
        pScaleData = nullptr;
    }

    if (pMirrorData != nullptr) {
        delete pMirrorData;
        pMirrorData = nullptr;
    }
    reset();
}

/**
 * 设置输入参数
 * @param width
 * @param height
 * @param pixelFormat
 */
void YuvConvertor::setInputParams(int width, int height, int pixelFormat) {
    mWidth = width;
    mHeight = height;
    mPixelFormat = pixelFormat;
    //LOGD("input video params:[%d, %d, %s]", width, height, av_get_pix_fmt_name(getPixelFormat((PixelFormat)pixelFormat)));
}

/**
 * 设置裁剪区域
 * @param x
 * @param y
 * @param width
 * @param height
 */
void YuvConvertor::setCrop(int x, int y, int width, int height) {
    mCropX = x;
    mCropY = y;
    mCropWidth = width;
    mCropHeight = height;
}

/**
 * 设置旋转角度
 * @param degree
 */
void YuvConvertor::setRotate(int degree) {
    switch (degree) {
        case 90: {
            mRotationMode = libyuv::kRotate90;
            break;
        }
        case 180: {
            mRotationMode = libyuv::kRotate180;
            break;
        }
        case 270: {
            mRotationMode = libyuv::kRotate270;
            break;
        }
        default: {
            mRotationMode = libyuv::kRotate0;
            break;
        }
    }
}

/**
 * 设置是否缩放
 * @param width
 * @param height
 */
void YuvConvertor::setScale(int width, int height) {
    mScaleWidth = width;
    mScaleHeight = height;
}

/**
 * 设置是否镜像
 * @param mirror
 */
void YuvConvertor::setMirror(bool mirror) {
    mMirror = mirror;
}

/**
 * 准备转换器
 * @return
 */
int YuvConvertor::prepare() {

    LOGE("YuvConvertor  prepare   mRotationMode: %d", mRotationMode);

    if ((mCropWidth == 0 || mCropHeight == 0)
        && (mRotationMode == libyuv::kRotate0)
        && (mScaleWidth == 0 && mScaleHeight == 0) &&!mMirror
        && (mPixelFormat == 4)) {
        LOGE("YuvConvertor  prepare   mNeedConvert: false");
        mNeedConvert = false;
        return -1;
    }

    LOGE("YuvConvertor  prepare  true");

    mNeedConvert = true;

    // 如果裁剪宽高不存在，则直接用源宽高处理
    if (mCropWidth == 0 && mCropHeight == 0) {
        mCropWidth = mWidth;
        mCropHeight = mHeight;
    }

    // 限定裁剪宽度为偶数
    if (mCropWidth % 2 == 1) {
        if (mCropHeight >= mCropWidth) {
            mCropHeight = (int) (1.0 * (mCropWidth - 1) / mCropWidth * mCropHeight);
            mCropHeight = mCropHeight % 2 == 1 ? mCropHeight - 1 : mCropHeight;
        }
        mCropWidth--;
    }

    // 限定裁剪高度为偶数
    if (mCropHeight % 2 == 1) {
        if (mCropWidth >= mCropHeight) {
            mCropWidth = (int) (1.0 * (mCropHeight - 1) / mCropHeight * mCropWidth);
            mCropWidth = mCropWidth % 2 == 1 ? mCropWidth - 1 : mCropWidth;
        }
        mCropHeight--;
    }

    // 创建裁剪yuv缓冲对象
    if (mCropWidth > 0 && mCropHeight > 0) {
        int width = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                    ? mCropWidth : mCropHeight;
        int height = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                     ? mCropHeight : mCropWidth;
        pCropData = new YuvData();
        pCropData->alloc(width, height);
    }

    // 创建缩放yuv缓冲对象
    if (mScaleWidth > 0 && mScaleHeight > 0) {
        pScaleData = new YuvData();
        pScaleData->alloc(mScaleWidth, mScaleHeight);
    }

    // 创建镜像yuv缓冲对象
    if (mMirror) {
        pMirrorData = new YuvData();
        pMirrorData->alloc(getOutputWidth(), getOutputHeight());
    }

    return 1;
}

void YuvConvertor::writeData(uint8_t *data, int len) {
    if(flag) {
        return ;
    }
    flag = count > 40;

    char name[32];
    //char* name = "/storage/emulated/0/1/ccc%d.yuv";
    sprintf(name, "/storage/emulated/0/1/ccc%d.yuv", count);
    FILE *file = fopen(name, "w");
    if(file == nullptr) {
        LOGE("file create fail");
        return ;
    }
    int ret = fwrite(data, 1, len, file);
    LOGE("YuvConvertor  writeData  ret: %d", ret);
    fclose(file);

    count++;
}

/**
 * 转换YUV数据
 * @param mediaData
 * @return
 */
int YuvConvertor::convert(AVMediaData *mediaData) {

    if (!mNeedConvert) {
        LOGE("Unable to convert media data");
        return -1;
    }

    if (mediaData->type != MediaVideo) {
        LOGE("Failed to conver current media data: %s", mediaData->getName());
        return -1;
    }

    if (mCropX + mCropWidth > mediaData->width || mCropY + mCropHeight > mediaData->height) {
        LOGE("crop argument invalid, media data: [%d, %d], crop: [%d, %d, %d, %d]",
                mediaData->width, mediaData->height,
                mCropX, mCropY, mCropWidth, mCropHeight);
        return -1;
    }

    //LOGE("convert mediaData.width: %d, mediaData.height: %d, mCropWidth: %d, mCropHeight: %d", mediaData->width,mediaData->height, mCropWidth, mCropHeight);

    //LOGE("convert  mediaData->width: %d, mediaData->height: %d, mCropWidth: %d, mCropHeight: %d, mRotationMode: %d",mediaData->width, mediaData->height, mCropWidth, mCropHeight, mRotationMode);

    int ret = ConvertToI420(mediaData->image, (size_t) mediaData->length,
                        pCropData->dataY, pCropData->lineSizeY,
                        pCropData->dataU, pCropData->lineSizeU,
                        pCropData->dataV, pCropData->lineSizeV,
                        mCropX, mCropY, mediaData->width, mediaData->height, mCropWidth, mCropHeight,
                        mRotationMode, libyuv::FOURCC_NV21);

    //LOGE("convert   ss ");

    if (ret < 0) {
        LOGE("Failed to call ConvertToI420: %d", ret);
        return ret;
    }
    YuvData *output = pCropData;
    int outputWidth = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180) ? mCropWidth : mCropHeight;
    int outputHeight = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180) ? mCropHeight : mCropWidth;

    fillMediaData(mediaData, output, outputWidth, outputHeight);

    //writeData(mediaData->image, mediaData->length);

    //writeData(mediaData->image, mediaData->length);

    //LOGE("YuvConvertor convert end   mediaData->width: %d, height: %d",mediaData->width, mediaData->height);

    return 0;
}

/**
 * 缩放处理
 * @param src
 * @param srcW
 * @param srcH
 * @return
 */
int YuvConvertor::scale(YuvData *src, int srcW, int srcH) {
    int ret;
    ret = libyuv::I420Scale(src->dataY, src->lineSizeY,
                            src->dataU, src->lineSizeU,
                            src->dataV, src->lineSizeV,
                            srcW, srcH,
                            pScaleData->dataY, pScaleData->lineSizeY,
                            pScaleData->dataU, pScaleData->lineSizeU,
                            pScaleData->dataV, pScaleData->lineSizeV,
                            mScaleWidth, mScaleHeight,
                            libyuv::kFilterBox);
    if (ret < 0) {
        LOGE("Failed to call I420Scale: %d", ret);
        return ret;
    }

    return 0;
}

/**
 * 镜像处理
 * @param src
 * @param srcW
 * @param srcH
 * @return
 */
int YuvConvertor::mirror(YuvData *src, int srcW, int srcH) {
    int ret;
    ret = libyuv::I420Mirror(src->dataY, src->lineSizeY,
                             src->dataU, src->lineSizeU,
                             src->dataV, src->lineSizeV,
                             pMirrorData->dataY, pMirrorData->lineSizeY,
                             pMirrorData->dataU, pMirrorData->lineSizeU,
                             pMirrorData->dataV, pMirrorData->lineSizeV,
                             srcW, srcH);
    if (ret < 0) {
        LOGE("Failed to call I420Mirror: %d", ret);
        return ret;
    }
    return 0;
}

/**
 * 填充媒体数据
 * @param model
 * @param src
 * @param srcW
 * @param srcH
 */
void YuvConvertor::fillMediaData(AVMediaData *model, YuvData *src, int srcW, int srcH) {
    uint8_t *image = new uint8_t[srcW * srcH * 3 / 2];
    if (model != nullptr) {
        model->free();
    } else {
        model = new AVMediaData();
    }
    model->image = image;
    memcpy(model->image, src->dataY, (size_t) srcW * srcH);
    memcpy(model->image + srcW * srcH, src->dataU, (size_t) srcW * srcH / 4);
    memcpy(model->image + srcW * srcH * 5 / 4, src->dataV, (size_t) srcW * srcH / 4);
    model->length = srcW * srcH * 3 / 2;
    model->width = srcW;
    model->height = srcH;
    model->pixelFormat = 4;
    model->type = MediaVideo;
}

/**
 * 获取输出宽度
 * @return
 */
int YuvConvertor::getOutputWidth() {
    int width = mScaleWidth;
    if (width == 0) {
        width = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                ? mCropWidth : mCropHeight;
    }
    return width;
}

/**
 * 获取输出高度
 * @return
 */
int YuvConvertor::getOutputHeight() {
    int height = mScaleHeight;
    if (height == 0) {
        height = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                 ? mCropHeight : mCropWidth;
    }
    return height;
}

