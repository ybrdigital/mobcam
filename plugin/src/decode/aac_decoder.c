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
#include "aac_decoder.h"

#include <libavcodec/avcodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mobcam_aac_decoder {
    const AVCodec *codec;
    AVCodecContext *codec_ctx;
    AVFrame *frame;
    AVPacket *pkt;
    mobcam_decoded_audio_cb_t on_audio;
    void *user_data;
};

mobcam_aac_decoder_t *mobcam_aac_decoder_create(mobcam_decoded_audio_cb_t on_audio, void *user_data) {
    mobcam_aac_decoder_t *dec = (mobcam_aac_decoder_t *)calloc(1, sizeof(mobcam_aac_decoder_t));
    if (!dec) return NULL;

    dec->on_audio = on_audio;
    dec->user_data = user_data;

    dec->codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (!dec->codec) {
        free(dec);
        return NULL;
    }

    dec->codec_ctx = avcodec_alloc_context3(dec->codec);
    if (!dec->codec_ctx) {
        free(dec);
        return NULL;
    }

    if (avcodec_open2(dec->codec_ctx, dec->codec, NULL) < 0) {
        avcodec_free_context(&dec->codec_ctx);
        free(dec);
        return NULL;
    }

    dec->frame = av_frame_alloc();
    dec->pkt = av_packet_alloc();

    return dec;
}

bool mobcam_aac_decoder_decode(mobcam_aac_decoder_t *dec, const uint8_t *adts_data, size_t adts_size, int64_t pts_us) {
    if (!dec || !dec->codec_ctx || !adts_data || adts_size == 0) return false;

    av_packet_unref(dec->pkt);
    dec->pkt->data = (uint8_t *)adts_data;
    dec->pkt->size = (int)adts_size;
    dec->pkt->pts = pts_us;
    dec->pkt->dts = pts_us;

    int ret = avcodec_send_packet(dec->codec_ctx, dec->pkt);
    if (ret < 0) {
        av_packet_unref(dec->pkt);
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec->codec_ctx, dec->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            break;
        }

        if (dec->on_audio && dec->frame->nb_samples > 0 && dec->frame->data[0]) {
            int channels = dec->frame->ch_layout.nb_channels;
            if (channels <= 0 && dec->codec_ctx) {
                channels = dec->codec_ctx->ch_layout.nb_channels;
            }
            if (channels <= 0) {
                channels = 2; // Default stereo fallback
            }

            int sample_rate = dec->frame->sample_rate;
            if ((sample_rate < 8000 || sample_rate > 192000) && dec->codec_ctx) {
                sample_rate = dec->codec_ctx->sample_rate;
            }
            if (sample_rate < 8000 || sample_rate > 192000) {
                sample_rate = 48000;
            }

            mobcam_audio_frame_t af;
            memset(&af, 0, sizeof(af));
            af.samples_per_sec = sample_rate;
            af.channels = channels;
            af.frames = (uint32_t)dec->frame->nb_samples;
            af.pts_us = (dec->frame->pts != AV_NOPTS_VALUE) ? dec->frame->pts : pts_us;

            if (dec->frame->format == AV_SAMPLE_FMT_FLTP) {
                af.format = 0; // AUDIO_FORMAT_FLOAT_PLANAR
                bool valid = true;
                for (int i = 0; i < channels && i < 8; i++) {
                    af.data[i] = dec->frame->data[i];
                    if (!af.data[i]) valid = false;
                }
                if (valid) dec->on_audio(dec->user_data, &af);
            } else if (dec->frame->format == AV_SAMPLE_FMT_FLT) {
                af.format = 1; // AUDIO_FORMAT_FLOAT (interleaved)
                af.data[0] = dec->frame->data[0];
                dec->on_audio(dec->user_data, &af);
            } else if (dec->frame->format == AV_SAMPLE_FMT_S16P) {
                af.format = 2; // AUDIO_FORMAT_16BIT_PLANAR
                bool valid = true;
                for (int i = 0; i < channels && i < 8; i++) {
                    af.data[i] = dec->frame->data[i];
                    if (!af.data[i]) valid = false;
                }
                if (valid) dec->on_audio(dec->user_data, &af);
            } else if (dec->frame->format == AV_SAMPLE_FMT_S16) {
                af.format = 3; // AUDIO_FORMAT_16BIT (interleaved)
                af.data[0] = dec->frame->data[0];
                dec->on_audio(dec->user_data, &af);
            }
        }
        av_frame_unref(dec->frame);
    }
    av_packet_unref(dec->pkt);
    return true;
}

void mobcam_aac_decoder_destroy(mobcam_aac_decoder_t *dec) {
    if (!dec) return;
    if (dec->codec_ctx) avcodec_free_context(&dec->codec_ctx);
    if (dec->frame) av_frame_free(&dec->frame);
    if (dec->pkt) av_packet_free(&dec->pkt);
    free(dec);
}
