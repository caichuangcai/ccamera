//
// Created by CainHuang on 2020/1/11.
//

#ifndef RESAMPLER_H
#define RESAMPLER_H

#include "MediaHeader.h"

/**
 * convert audio pcm data to expected value
 */
class Resampler {
public:
    Resampler();

    virtual ~Resampler();

    void release();

    // set input pcm params
    void setInput(int sample_rate, int channels, AVSampleFormat sample_fmt, AVRational time_base);

    // set output pcm params
    void setOutput(int sample_rate, uint64_t channel_layout, AVSampleFormat sample_fmt,
                   int channels, int nb_samples);

    // init convertor
    int init();

    // convert data
    int resample(const uint8_t *data, AVCodecContext *codecCtx);

    // get converted frame
    AVFrame *getConvertedFrame();

    AVSampleFormat getInputSampleFormat();

    uint64_t samples_count = 0;                 // sample numbers

private:
    SwrContext *pSampleConvertCtx = nullptr;  // resample context
    AVFrame *mSampleFrame = nullptr;          // converted frame
    uint8_t **mSampleBuffer = nullptr;        // resample buffer

    int mOutSampleSize = 0;             // output sample size
    int mOutSampleRate = 0;             // output sample rate
    int64_t mOutChannelLayout = 0;      // output channel layout
    AVSampleFormat mOutSampleFormat;// output sample format
    int mOutFrameSize = 0;              // output frame size
    int mOutChannels = 0;               // output channels

    int mInSampleRate = 0;              // input sample rate
    int mInChannels = 0;                // input channels
    int64_t mInChannelLayout;       // input channel layout
    AVSampleFormat mInSampleFormat; // input sample format
};


#endif //RESAMPLER_H
