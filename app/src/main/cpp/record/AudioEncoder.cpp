//
// Created by Administrator on 2022/1/21.
//

#include "AudioEncoder.h"

AudioEncoder::AudioEncoder() {

}

void AudioEncoder::setFormatContext(AVFormatContext *_format_ctx) {
    pFormatCtx = _format_ctx;
}

void AudioEncoder::createEncoder() {

    AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_AAC);

    if(encoder == nullptr) {
        LOGE("Failed to find encoder: AV_CODEC_ID_AAC");
        return ;
    }

    //LOGE("createEncoder  name:%s, id:%d", encoder->name, encoder->id);

    pCodec = encoder;
    // 创建编码上下文
    pCodecCtx = avcodec_alloc_context3(encoder);

    if (!pCodecCtx) {
        LOGE("Failed to allocate the encoder context");
        return ;
    }

    /*if((pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)) {
        pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }*/

    pStream = avformat_new_stream(pFormatCtx, encoder);
    if (!pStream) {
        LOGE("Failed to allocate stream.");
        return ;
    }

    pStream->id = pFormatCtx->nb_streams-1;

    vPacket = av_packet_alloc();
    if(!vPacket) {
        LOGE("VideoEncoder  av_packet_alloc() failed");
    }
}

void AudioEncoder::setAudioParams(int bitrate, int sampleRate, int channels) {
    pCodecCtx->sample_rate = sampleRate;
    pCodecCtx->channels = channels;
    pCodecCtx->bit_rate = bitrate;
    pCodecCtx->channel_layout = (uint64_t) av_get_default_channel_layout(channels);
    pCodecCtx->sample_fmt = pCodec->sample_fmts[0];
    pCodecCtx->time_base = AVRational{1, pCodecCtx->sample_rate};
}

void AudioEncoder::openEncoder(std::map<std::string, std::string> mEncodeOptions) {

    // 设置时钟基准
    pStream->time_base = pCodecCtx->time_base;

    // 打开编码器
    int ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret < 0) {
        return ;
    }

    // 将编码器参数复制到媒体流中
    ret = avcodec_parameters_from_context(pStream->codecpar, pCodecCtx);
    if (ret < 0) {
        LOGE("Failed to copy encoder parameters to video stream");
        return ;
    }

}

AVCodecContext* AudioEncoder::getContext() {
    return pCodecCtx;
}

int AudioEncoder::encodeFrame(AVFrame *frame) {
    int ret = 0;

    // 送去编码
    ret = avcodec_send_frame(pCodecCtx, frame);
    if (ret < 0) {
        LOGE("AudioEncoder  failed to call avcodec_send_frame: %s   ret: %d", av_err2str(ret), ret);
        return 0;
    }

    while (ret >= 0) {
        // 取出编码后的数据包
        ret = avcodec_receive_packet(pCodecCtx, vPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0) {
            LOGE("Failed to call avcodec_receive_packet: %s, type: %s", av_err2str(ret), "Video");
            //av_packet_unref(&packet);
            return ret;
        }

        // 计算输出的pts
        av_packet_rescale_ts(vPacket, pCodecCtx->time_base, pStream->time_base);
        vPacket->stream_index = pStream->index;

        ret = av_interleaved_write_frame(pFormatCtx, vPacket);
        if (ret < 0) {
            LOGE("Failed to call av_interleaved_write_frame: %s, type: %s", av_err2str(ret), "Video");
            //av_packet_unref(&packet);
            return ret;
        }
        LOGE("write packet: type:%s, pts: %lld, s: %f", "Audio", vPacket->pts, vPacket->pts * av_q2d(pStream->time_base));
    }

    //av_packet_unref(&packet);
    ret = ret == AVERROR_EOF ? 1 : 0;
    //LOGE("Audio encode frame  ret:%s", ret?"true":"false");
    return ret;
}

void AudioEncoder::closeEncoder() {
    if (pCodecCtx != nullptr) {
        avcodec_close(pCodecCtx);
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = nullptr;
        pCodec = nullptr;
    }
    if (pStream != nullptr && pStream->metadata) {
        av_dict_free(&pStream->metadata);
        pStream->metadata = nullptr;
    }
    pStream = nullptr;
}

void AudioEncoder::reset() {

}

AudioEncoder::~AudioEncoder() {

}