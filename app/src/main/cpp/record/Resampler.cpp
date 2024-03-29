#include "Resampler.h"

Resampler::Resampler() {
    pSampleConvertCtx = nullptr;
    mSampleFrame = nullptr;

    mOutSampleSize = 0;
    mOutSampleRate = 0;
    mOutChannelLayout = 0;
    mOutSampleFormat = AV_SAMPLE_FMT_NONE;
    mOutFrameSize = 0;
    mOutChannels = 0;

    mInSampleRate = 0;
    mInChannels = 0;
    mInChannelLayout = 0;
    mInSampleFormat = AV_SAMPLE_FMT_NONE;
}

Resampler::~Resampler() {
    release();
}

void Resampler::release() {
    if (mSampleFrame != nullptr) {
        av_frame_free(&mSampleFrame);
        mSampleFrame = nullptr;
    }
    if (mSampleBuffer != nullptr) {
        for (int i = 0; i < mOutChannels; i++) {
            if (mSampleBuffer[i] != nullptr) {
                av_free(mSampleBuffer[i]);
                mSampleBuffer[i] = nullptr;
            }
        }
        delete[] mSampleBuffer;
        mSampleBuffer = nullptr;
    }
}

/**
 * 设置输入参数
 * @param sample_rate
 * @param channel_layout
 * @param sample_fmt
 */
void Resampler::setInput(int sample_rate, int channels, AVSampleFormat sample_fmt, AVRational time_base) {
    mInSampleRate = sample_rate;
    mInChannels = av_sample_fmt_is_planar(sample_fmt) ? channels : 1;
    mInSampleFormat = sample_fmt;
    mInChannelLayout = av_get_default_channel_layout(mInChannels);
}

/**
 * 设置输出音频参数
 * @param sample_rate
 * @param channel_layout
 * @param sample_fmt
 * @param channels
 * @param nb_samples
 */
void Resampler::setOutput(int sample_rate, uint64_t channel_layout, AVSampleFormat sample_fmt,
                          int channels, int nb_samples) {

    mOutSampleRate = sample_rate;
    mOutChannelLayout = channel_layout;
    mOutSampleFormat = sample_fmt;
    mOutFrameSize = nb_samples;
    mOutChannels = channels;

    mSampleFrame = av_frame_alloc();
    mSampleFrame->format = sample_fmt;
    mSampleFrame->sample_rate = mOutSampleRate;
    mSampleFrame->channels = mOutChannels;
    mSampleFrame->nb_samples = nb_samples;
    mSampleFrame->channel_layout = (uint64_t)channel_layout;
    mSampleFrame->pts = 0;

    // 获取声道数
    mOutChannels = av_sample_fmt_is_planar(sample_fmt) ? channels : 1;
    // 计算出缓冲区大小
    mOutSampleSize = av_samples_get_buffer_size(nullptr, channels,
                                                nb_samples,
                                                sample_fmt, 1) / mOutChannels;
    // 初始化采样缓冲区大小
    mSampleBuffer = new uint8_t *[mOutChannels];
    for (int i = 0; i < mOutChannels; i++) {
        mSampleBuffer[i] = (uint8_t *) av_malloc((size_t) mOutSampleSize);
    }
}

/**
 * 初始化重采样器
 * @return
 */
int Resampler::init() {
    pSampleConvertCtx = swr_alloc();
    if(!pSampleConvertCtx) {
        LOGE("Could not allocate resampler context");
        return -1;
    }
    av_opt_set_int(pSampleConvertCtx, "in_channel_layout",    mInChannelLayout, 0);
    av_opt_set_int(pSampleConvertCtx, "in_sample_rate",       mInSampleRate, 0);
    av_opt_set_sample_fmt(pSampleConvertCtx, "in_sample_fmt", mInSampleFormat, 0);

    av_opt_set_int(pSampleConvertCtx, "out_channel_layout",    mOutChannelLayout, 0);
    av_opt_set_int(pSampleConvertCtx, "out_sample_rate",       mOutSampleRate, 0);
    av_opt_set_sample_fmt(pSampleConvertCtx, "out_sample_fmt", mOutSampleFormat, 0);

    int ret = 0;

    if ((ret = swr_init(pSampleConvertCtx)) < 0) {
        LOGE("Failed to initialize the resampling context");
        return ret;
    }
    return 0;
}

/**
 * resample pcm data
 * @param data          pcm data
 * @param nb_samples    pcm data length
 * @return number of samples output per channel, negative value on error
 */
int Resampler::resample(const uint8_t *data, AVCodecContext *codecCtx) {
    int ret = 0;
    // 如果输入输出不相等，则进行转码再做处理
    if (mInChannels != mOutChannels || mOutSampleFormat != mInSampleFormat || mOutSampleRate != mInSampleRate)
    {

        int dst_nb_samples = av_rescale_rnd(swr_get_delay(pSampleConvertCtx, codecCtx->sample_rate) + mSampleFrame->nb_samples,
                                        codecCtx->sample_rate, codecCtx->sample_rate, AV_ROUND_UP);

        ret = swr_convert(pSampleConvertCtx, mSampleBuffer, mOutFrameSize,
                              &data, mSampleFrame->nb_samples);
        if (ret < 0) {
            LOGE("swr_convert error: %s", av_err2str(ret));
            return -1;
        }
        // 将数据复制到采样帧中
        /*int ret = avcodec_fill_audio_frame(mSampleFrame, mOutChannels, mOutSampleFormat, mSampleBuffer[0],
                                 mOutSampleSize, 0);
        if(ret < 0) {
            LOGE("avcodec_fill_audio_frame  fail  ret: %s, %d", av_err2str(ret), ret);
        }*/

        for (int i = 0; i < mOutChannels; i++) {
            mSampleFrame->data[i] = mSampleBuffer[i];
            mSampleFrame->linesize[i] = mOutSampleSize;
        }

        mSampleFrame->pts = av_rescale_q(samples_count, (AVRational){1, codecCtx->sample_rate}, codecCtx->time_base);
        samples_count += dst_nb_samples;

    } else {

        // 直接将数据复制到采样帧中
        ret = av_samples_fill_arrays(mSampleFrame->data, mSampleFrame->linesize, data,
                                     mOutChannels, mSampleFrame->nb_samples,
                                     mOutSampleFormat, 1);
        if (ret < 0) {
            LOGE("Failed to call av_samples_fill_arrays: %s", av_err2str(ret));
            return -1;
        }

        mSampleFrame->pts = av_rescale_q(samples_count, (AVRational){1, codecCtx->sample_rate}, codecCtx->time_base);
        samples_count += mSampleFrame->nb_samples;
    }
    return 0;
}

/**
 * get resampled frame
 * @return
 */
AVFrame* Resampler::getConvertedFrame() {
    return mSampleFrame;
}

AVSampleFormat Resampler::getInputSampleFormat() {
    return mInSampleFormat;
}