//
// Created by Administrator on 2022/1/20.
//

#include "MediaMuxer.h"

MediaMuxer::MediaMuxer() {

}

void MediaMuxer::init() {

}

void MediaMuxer::setOutputPath(const char* path) {
    outputPath = av_strdup(path);
}

void MediaMuxer::startRecord() {

}

void MediaMuxer::stopRecord() {

}

int MediaMuxer::prepare() {

    av_register_all();

    avformat_network_init();

    LOGE("MediaMuxer outputPath: %s", outputPath);

    int ret = avformat_alloc_output_context2(&pFormatCtx, nullptr, "flv", outputPath);

    if (!pFormatCtx || ret < 0) {
        LOGE("AVMediaMuxer - failed to call avformat_alloc_output_context2: %s", av_err2str(ret));
        return AVERROR_UNKNOWN;
    }

    LOGE("prepare  video_codec: %d, audio_codec: %d", pFormatCtx->oformat->video_codec,  pFormatCtx->oformat->audio_codec);

    // 打开视频编码器
    openVideoEncoder();

    LOGE("video encoder is ready.....");

    // 打开音频编码器
    openAudioEncoder();

    LOGE("audio encoder is ready.....");

    mImageFrame = av_frame_alloc();
    if (!mImageFrame) {
        LOGE("Failed to allocate video frame");
        return -1;
    }
    mImageFrame->format = mPixelFormat;
    mImageFrame->width = videoWidth;
    mImageFrame->height = videoHeight;
    mImageFrame->pts = 0;

    int size = av_image_get_buffer_size(mPixelFormat, videoWidth, videoHeight, 1);
    if (size < 0) {
        LOGE("Failed to get image buffer size: %s", av_err2str(size));
        return -1;
    }

    mImageBuffer = (uint8_t *) av_malloc((size_t) size);
    if (!mImageBuffer) {
        LOGE("Failed to allocate image buffer");
        return -1;
    }

    AVCodecContext *pAudioCodecCtx = audioEncoder->getContext();
    mResampler = new Resampler();
    mResampler->setInput(sampleRate, channels, mSampleFormat, pAudioCodecCtx->time_base);

    LOGE("Audio output info    sample_rate: %d, channel_layout: %llu, sample_fmt: %d, channels: %d, frame_size: %d", pAudioCodecCtx->sample_rate,
         pAudioCodecCtx->channel_layout,
         pAudioCodecCtx->sample_fmt,
         pAudioCodecCtx->channels,
         pAudioCodecCtx->frame_size);

    mResampler->setOutput(pAudioCodecCtx->sample_rate, pAudioCodecCtx->channel_layout,
                          pAudioCodecCtx->sample_fmt, pAudioCodecCtx->channels, pAudioCodecCtx->frame_size);
    ret = mResampler->init();
    if (ret < 0) {
        LOGE("Failed to init audio convertor.");
        return ret;
    }
    LOGE("Audio Resampler is ready.....");


    if (!(pFormatCtx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&pFormatCtx->pb, outputPath, AVIO_FLAG_WRITE)) < 0) {
            LOGE("AVMediaMuxer - Failed to open output file '%s'", outputPath);
            return ret;
        }
    }

    ret = avformat_write_header(pFormatCtx, nullptr);
    if (ret < 0) {
        LOGE("AVMediaMuxer - Failed to call avformat_write_header: %s,  ret: %d", av_err2str(ret), ret);
        return ret;
    }

    LOGE("MediaMuxer is ready.....");

    return 0;
}

int MediaMuxer::openVideoEncoder() {
    videoEncoder = new VideoEncoder();
    videoEncoder->setFormatContext(pFormatCtx);
    // 创建编码器
    videoEncoder->createEncoder();
    // 设置视频参数
    videoEncoder->setVideoParams(videoWidth, videoHeight, mPixelFormat, videoFrameRate, 0);
    // 打开编码器
    videoEncoder->openEncoder(mEncodeOptions);
    videoCodecCtx = videoEncoder->getCodecContext();
    return 0;
}

int MediaMuxer::openAudioEncoder() {
    audioEncoder = new AudioEncoder();
    audioEncoder->setFormatContext(pFormatCtx);
    // 创建编码器
    audioEncoder->createEncoder();
    // 设置音频参数
    audioEncoder->setAudioParams(AUDIO_BIT_RATE, sampleRate, channels);
    // 打开编码器
    audioEncoder->openEncoder(mEncodeOptions);
    audioCodecCtx = audioEncoder->getContext();
    return 0;
}

/***
 * 判断视频跟音频的时间戳对比，判断编码视频帧还是音频帧
 * return true：编码视频帧  false：编码音频帧
 * */
bool MediaMuxer::encodeVideoNow() {
    return av_compare_ts(video_next_pts, videoCodecCtx->time_base,
                         mResampler->samples_count, audioCodecCtx->time_base) <= 0;
}

int MediaMuxer::encodeMediaData(AVMediaData *mediaData) {
    int ret = 0;

    bool isVideo = mediaData->type == MediaVideo;

    AVFrame *frame;

    if(isVideo)
    {
        /* check if we want to generate more frames */
        /*if (av_compare_ts(video_next_pts, videoCodecCtx->time_base,
                          10.0, (AVRational){ 1, 1 }) > 0) {
            return 1;
        }*/
        frame = mImageFrame;
    }
    else {
        /* check if we want to generate more frames */
        /*if (av_compare_ts(mResampler->samples_count, audioCodecCtx->time_base,
                          10.0, (AVRational){ 1, 1 }) > 0) {
            return 1;
        }*/
        frame = mResampler->getConvertedFrame();
    }

    uint8_t *data = isVideo ? mediaData->image : mediaData->sample;

    // 填充数据
    if (data != nullptr)
    {
        ret = isVideo ? fillImage(mediaData) : fillSample(mediaData);
        if (ret < 0) {
            return 1;
        }
    }

    if (isVideo) {
        return videoEncoder->encodeFrame(frame);
    } else {
        return audioEncoder->encodeFrame(frame);
    }
}

int MediaMuxer::fillImage(AVMediaData *data) {
    int ret;
    ret = av_image_fill_arrays(mImageFrame->data, mImageFrame->linesize, data->image,
                               AV_PIX_FMT_YUV420P,
                               data->width, data->height, 1);
    if (ret < 0) {
        LOGE("MediaMuxer  fillImage  ret: %d, %s", ret, av_err2str(ret));
        return -1;
    }
    mImageFrame->pts = video_next_pts++;
    return 0;
}

int MediaMuxer::fillSample(AVMediaData *data) {
    if (mResampler != nullptr) {
        int ret = mResampler->resample(data->sample, audioCodecCtx);
        if (ret < 0) {
            LOGE("resample error!");
        }
    }
    return 0;
}

/**
 * 添加编码参数
 * @param key
 * @param value
 */
void MediaMuxer::addEncodeOptions(std::string key, std::string value) {
    mEncodeOptions[key] = value;
}

/**
 * 设置质量系数
 * @param quality
 */
void MediaMuxer::setQuality(int quality) {
    std::stringstream ss;
    ss << quality;
    std::string str;
    ss >> str;
    mEncodeOptions["crf"] = str;
}

/**
 * 刷新缓冲区
 * */
void MediaMuxer::flush() {
    audioEncoder->encodeFrame(NULL);
    videoEncoder->encodeFrame(NULL);
}

/**
 * 停止录制，写文件尾
 * */
int MediaMuxer::stop() {
    // 写入文件尾
    int ret = av_write_trailer(pFormatCtx);
    if (ret < 0) {
        LOGE("AVMediaMuxer -Failed to call av_write_trailer: %s", av_err2str(ret));
        return ret;
    }
    LOGE("AVMediaMuxer - av_write_trailer() success");
    return 0;
}

void MediaMuxer::release() {
    if (mImageFrame != nullptr) {
        av_frame_free(&mImageFrame);
        mImageFrame = nullptr;
    }
    if (mImageBuffer != nullptr) {
        av_free(mImageBuffer);
        mImageBuffer = nullptr;
    }
    if (audioEncoder != nullptr) {
        audioEncoder->closeEncoder();
        audioEncoder->reset();
        delete audioEncoder;
        audioEncoder = nullptr;
    }
    if (videoEncoder != nullptr) {
        videoEncoder->closeEncoder();
        delete videoEncoder;
        videoEncoder = nullptr;
    }
    if (mResampler != nullptr) {
        mResampler->release();
        delete mResampler;
        mResampler = nullptr;
    }
    LOGE("MediaMuxer release  %s", mResampler== nullptr?"mResampler is null":"mResampler not null");
}

MediaMuxer::~MediaMuxer() {
    release();
}