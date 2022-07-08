//
// Created by Administrator on 2022/1/21.
//

#include "VideoEncoder.h"

VideoEncoder::VideoEncoder() {



}

void VideoEncoder::setFormatContext(AVFormatContext *_format_ctx) {
    pFormatCtx = _format_ctx;
}

AVCodecContext* VideoEncoder::getCodecContext() {
    return pCodecCtx;
}

void VideoEncoder::setVideoParams(int width, int height, AVPixelFormat pixelFormat, int frameRate, int maxBitRate) {
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->pix_fmt = pixelFormat;
    pCodecCtx->gop_size = 10; // frameRate
    pCodecCtx->framerate = (AVRational) {frameRate, 1};
    pStream->time_base = (AVRational) {1, frameRate};
    pCodecCtx->time_base = (AVRational) {1, frameRate};

    // 设置最大比特率
    pCodecCtx->rc_max_rate = 1024 * 1024;
    pCodecCtx->rc_min_rate = 256 * 1000;
    pCodecCtx->rc_buffer_size = 10 * 1024 * 1024;
    pCodecCtx->max_b_frames = 1;
}

void VideoEncoder::createEncoder() {

    AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_H264);

    if(encoder == nullptr) {
        LOGE("Failed to find encoder: AV_CODEC_ID_H264");
        return ;
    }

    pCodec = encoder;
    // 创建编码上下文

    pStream = avformat_new_stream(pFormatCtx, encoder);
    if (!pStream) {
        LOGE("Failed to allocate stream.");
        return ;
    }

    pStream->id = pFormatCtx->nb_streams-1;

    pCodecCtx = avcodec_alloc_context3(encoder);
    if (!pCodecCtx) {
        LOGE("Failed to allocate the encoder context");
        return ;
    }

    LOGE("CreateVideoEncoder codec_id: %d, codec_type: %d, width: %d, height: %d ", pCodecCtx->codec_id, pCodecCtx->codec_type, pCodecCtx->width, pCodecCtx->height);

    if((pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER))
    {
        pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        pStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    vPacket = av_packet_alloc();
    if(!vPacket) {
        LOGE("VideoEncoder  av_packet_alloc() failed");
    }
}

void VideoEncoder::openEncoder(std::map<std::string, std::string> mEncodeOptions) {

    // 设置时钟基准
    pStream->time_base = pCodecCtx->time_base;

    AVDictionary *param = 0;
    if(pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&param, "preset", "veryfast", 0);
    }

    // 打开编码器
    int ret = avcodec_open2(pCodecCtx, pCodec, &param);
    if (ret < 0) {
        av_dict_free(&param);
        LOGE("openVideoEncoder  ret: %d, %s", ret, av_err2str(ret));
        return ;
    }
    av_dict_free(&param);

    // 将编码器参数复制到媒体流中
    ret = avcodec_parameters_from_context(pStream->codecpar, pCodecCtx);
    if (ret < 0) {
        LOGE("Failed to copy encoder parameters to video stream");
        return ;
    }
}

int VideoEncoder::encodeFrame(AVFrame *frame) {
    // 视频编码
    int ret = avcodec_send_frame(pCodecCtx, frame);
    if (ret < 0) {
        LOGE("Fail avcodec_send_frame: %s, ret: %d", av_err2str(ret), ret);
        return 1;
    }

    while (ret >= 0)
    {
        // 编码后的数据包
        ret = avcodec_receive_packet(pCodecCtx, vPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0) {
            LOGE("Failed to call avcodec_receive_packet: %s, type: %s", av_err2str(ret), "Video");
            return ret;
        }

        // 计算输出视频数据的pts
        av_packet_rescale_ts(vPacket, pCodecCtx->time_base, pStream->time_base);
        vPacket->stream_index = pStream->index;

        ret = av_interleaved_write_frame(pFormatCtx, vPacket);
        if (ret < 0) {
            LOGE("Failed to call av_interleaved_write_frame: %s, type: %s", av_err2str(ret), "Video");
            return ret;
        }
        LOGE("write packet: type:%s, pts: %lld, s: %lf", "Video", vPacket->pts, vPacket->pts * av_q2d(pStream->time_base));
    }
    ret = ret == AVERROR_EOF ? 1 : 0;
    return ret;
}

void VideoEncoder::closeEncoder() {
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

VideoEncoder::~VideoEncoder() {

}