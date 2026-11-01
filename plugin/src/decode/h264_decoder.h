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
#ifndef MOBCAM_H264_DECODER_H
#define MOBCAM_H264_DECODER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mobcam_video_frame {
    int width;
    int height;
    int format; // 0 = YUV420P, 1 = NV12
    uint8_t *data[4];
    int linesize[4];
    int64_t pts_us;
    uint64_t timestamp_ns;
} mobcam_video_frame_t;

typedef void (*mobcam_decoded_video_cb_t)(void *user_data, const mobcam_video_frame_t *frame);

typedef struct mobcam_h264_decoder mobcam_h264_decoder_t;

mobcam_h264_decoder_t *mobcam_h264_decoder_create(const char *codec_name, mobcam_decoded_video_cb_t on_frame, void *user_data);

bool mobcam_h264_decoder_decode(mobcam_h264_decoder_t *dec, const uint8_t *nal_data, size_t nal_size, int64_t pts_us);

void mobcam_h264_decoder_destroy(mobcam_h264_decoder_t *dec);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_H264_DECODER_H
