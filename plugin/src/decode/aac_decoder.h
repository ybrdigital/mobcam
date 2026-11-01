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
#ifndef MOBCAM_AAC_DECODER_H
#define MOBCAM_AAC_DECODER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mobcam_audio_frame {
    int samples_per_sec;
    int channels;
    int format; // 0 = float planar, 1 = float interleaved, 2 = s16 planar, 3 = s16 interleaved
    uint8_t *data[8];
    uint32_t frames;
    int64_t pts_us;
    uint64_t timestamp_ns;
} mobcam_audio_frame_t;

typedef void (*mobcam_decoded_audio_cb_t)(void *user_data, const mobcam_audio_frame_t *frame);

typedef struct mobcam_aac_decoder mobcam_aac_decoder_t;

mobcam_aac_decoder_t *mobcam_aac_decoder_create(mobcam_decoded_audio_cb_t on_audio, void *user_data);

bool mobcam_aac_decoder_decode(mobcam_aac_decoder_t *dec, const uint8_t *adts_data, size_t adts_size, int64_t pts_us);

void mobcam_aac_decoder_destroy(mobcam_aac_decoder_t *dec);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_AAC_DECODER_H
