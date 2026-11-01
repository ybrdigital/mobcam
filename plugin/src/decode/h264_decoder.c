/*
 * MobCam - OBS Studio Plugin
 * Copyright (C) 2026 Yash Rayjada (YBR Digital)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "h264_decoder.h"

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mobcam_h264_decoder {
    enum AVCodecID current_codec_id;
    const AVCodec *codec;
    AVCodecContext *codec_ctx;
    AVCodecParserContext *parser;
    AVFrame *frame;
    AVPacket *pkt;
    mobcam_decoded_video_cb_t on_frame;
    void *user_data;
};

static bool init_codec(mobcam_h264_decoder_t *dec, enum AVCodecID cid) {
    if (dec->codec_ctx && dec->current_codec_id == cid) return true;

    if (dec->parser) {
        av_parser_close(dec->parser);
        dec->parser = NULL;
    }
    if (dec->codec_ctx) {
        avcodec_free_context(&dec->codec_ctx);
        dec->codec_ctx = NULL;
    }

    dec->codec = avcodec_find_decoder(cid);
    if (!dec->codec) {
        return false;
    }

    dec->parser = av_parser_init(cid);
    if (!dec->parser) {
        return false;
    }

    dec->codec_ctx = avcodec_alloc_context3(dec->codec);
    if (!dec->codec_ctx) {
        av_parser_close(dec->parser);
        dec->parser = NULL;
        return false;
    }

    // Set low-latency decoding flags
    dec->codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    dec->codec_ctx->flags2 |= (AV_CODEC_FLAG2_FAST | AV_CODEC_FLAG2_SHOW_ALL);
    dec->codec_ctx->thread_count = 1; // 1 thread eliminates multi-frame buffer delay
    dec->codec_ctx->has_b_frames = 0;

    if (avcodec_open2(dec->codec_ctx, dec->codec, NULL) < 0) {
        avcodec_free_context(&dec->codec_ctx);
        av_parser_close(dec->parser);
        dec->codec_ctx = NULL;
        dec->parser = NULL;
        return false;
    }

    dec->current_codec_id = cid;
    return true;
}

mobcam_h264_decoder_t *mobcam_h264_decoder_create(const char *codec_name, mobcam_decoded_video_cb_t on_frame, void *user_data) {
    mobcam_h264_decoder_t *dec = (mobcam_h264_decoder_t *)calloc(1, sizeof(mobcam_h264_decoder_t));
    if (!dec) return NULL;

    dec->on_frame = on_frame;
    dec->user_data = user_data;

    enum AVCodecID cid = (codec_name && (strcmp(codec_name, "hevc") == 0 || strcmp(codec_name, "h265") == 0))
                         ? AV_CODEC_ID_HEVC : AV_CODEC_ID_H264;

    if (!init_codec(dec, cid)) {
        free(dec);
        return NULL;
    }

    dec->frame = av_frame_alloc();
    dec->pkt = av_packet_alloc();

    return dec;
}

bool mobcam_h264_decoder_decode(mobcam_h264_decoder_t *dec, const uint8_t *nal_data, size_t nal_size, int64_t pts_us) {
    if (!dec || !nal_data || nal_size == 0) return false;

    // Robust parameter-set codec detection (only triggered on unambiguous parameter sets: HEVC VPS or H.264 SPS)
    if (nal_size >= 6) {
        int sc_len = 0;
        if (nal_data[0] == 0 && nal_data[1] == 0 && nal_data[2] == 1) sc_len = 3;
        else if (nal_data[0] == 0 && nal_data[1] == 0 && nal_data[2] == 0 && nal_data[3] == 1) sc_len = 4;

        if (sc_len > 0 && nal_size >= (size_t)sc_len + 2) {
            uint8_t b0 = nal_data[sc_len];
            uint8_t b1 = nal_data[sc_len + 1];

            // HEVC VPS parameter set: byte0 = 0x40 (type 32), byte1 = 0x01 (nuh_layer_id=0, tid_plus1=1)
            // Or HEVC SPS parameter set: byte0 = 0x42 (type 33), byte1 = 0x01
            if ((b0 == 0x40 || b0 == 0x42) && (b1 & 0x1F) >= 1) {
                if (dec->current_codec_id != AV_CODEC_ID_HEVC) {
                    init_codec(dec, AV_CODEC_ID_HEVC);
                }
            } else if (b0 == 0x67) { // H.264 SPS parameter set: nal_ref_idc=3, type=7 (0x67)
                if (dec->current_codec_id != AV_CODEC_ID_H264) {
                    init_codec(dec, AV_CODEC_ID_H264);
                }
            }
        }
    }

    if (!dec->codec_ctx) return false;

    av_packet_unref(dec->pkt);
    dec->pkt->data = (uint8_t *)nal_data;
    dec->pkt->size = (int)nal_size;
    dec->pkt->pts = pts_us;
    dec->pkt->dts = pts_us;

    int ret = avcodec_send_packet(dec->codec_ctx, dec->pkt);
    if (ret < 0) {
        // Fallback to parser in case frame has multi-NAL splitting needed
        const uint8_t *data_ptr = nal_data;
        int remaining = (int)nal_size;
        while (remaining > 0) {
            int parsed = av_parser_parse2(dec->parser, dec->codec_ctx,
                                          &dec->pkt->data, &dec->pkt->size,
                                          data_ptr, remaining,
                                          pts_us, pts_us, 0);
            data_ptr += parsed;
            remaining -= parsed;
            if (dec->pkt->size > 0) {
                dec->pkt->pts = pts_us;
                int r2 = avcodec_send_packet(dec->codec_ctx, dec->pkt);
                if (r2 >= 0) {
                    while (avcodec_receive_frame(dec->codec_ctx, dec->frame) >= 0) {
                        if (dec->on_frame && dec->frame->width > 0 && dec->frame->width <= 7680 &&
                            dec->frame->height > 0 && dec->frame->height <= 4320 && dec->frame->data[0]) {
                            mobcam_video_frame_t vf;
                            memset(&vf, 0, sizeof(vf));
                            vf.width = dec->frame->width;
                            vf.height = dec->frame->height;
                            vf.format = (dec->frame->format == AV_PIX_FMT_NV12) ? 1 : 0;
                            for (int i = 0; i < 4; i++) {
                                vf.data[i] = dec->frame->data[i];
                                vf.linesize[i] = dec->frame->linesize[i];
                            }
                            vf.pts_us = (dec->frame->pts != AV_NOPTS_VALUE) ? dec->frame->pts : pts_us;
                            dec->on_frame(dec->user_data, &vf);
                        }
                        av_frame_unref(dec->frame);
                    }
                }
            }
        }
        av_packet_unref(dec->pkt);
        return true;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec->codec_ctx, dec->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            break;
        }

        if (dec->on_frame && dec->frame->width > 0 && dec->frame->width <= 7680 &&
            dec->frame->height > 0 && dec->frame->height <= 4320 && dec->frame->data[0]) {
            mobcam_video_frame_t vf;
            memset(&vf, 0, sizeof(vf));
            vf.width = dec->frame->width;
            vf.height = dec->frame->height;
            vf.format = (dec->frame->format == AV_PIX_FMT_NV12) ? 1 : 0;

            for (int i = 0; i < 4; i++) {
                vf.data[i] = dec->frame->data[i];
                vf.linesize[i] = dec->frame->linesize[i];
            }
            vf.pts_us = (dec->frame->pts != AV_NOPTS_VALUE) ? dec->frame->pts : pts_us;
            dec->on_frame(dec->user_data, &vf);
        }
        av_frame_unref(dec->frame);
    }
    av_packet_unref(dec->pkt);
    return true;
}

void mobcam_h264_decoder_destroy(mobcam_h264_decoder_t *dec) {
    if (!dec) return;
    if (dec->parser) av_parser_close(dec->parser);
    if (dec->codec_ctx) avcodec_free_context(&dec->codec_ctx);
    if (dec->frame) av_frame_free(&dec->frame);
    if (dec->pkt) av_packet_free(&dec->pkt);
    free(dec);
}
