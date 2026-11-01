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
#ifndef MOBCAM_WATERMARK_OVERLAY_H
#define MOBCAM_WATERMARK_OVERLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <obs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mobcam_watermark_backup {
    uint8_t *y_buf;
    uint8_t *u_buf;
    uint8_t *v_buf;
    size_t capacity;
    int x;
    int y;
    int w;
    int h;
    int format;
    bool active;
} mobcam_watermark_backup_t;

/**
 * Draws the clean static "MobCam" watermark centered on the frame.
 * Backs up pristine pixels under the watermark before drawing to protect reference frames.
 */
void mobcam_draw_free_watermark(struct obs_source_frame *obs_frame, float opacity, mobcam_watermark_backup_t *backup);

/**
 * Restores the original background pixels back into the frame.
 * Must be called immediately after obs_source_output_video() so FFmpeg's DPB reference buffer is never polluted.
 */
void mobcam_restore_watermark_background(struct obs_source_frame *obs_frame, mobcam_watermark_backup_t *backup);

/**
 * Frees internal backup buffer memory.
 */
void mobcam_watermark_backup_free(mobcam_watermark_backup_t *backup);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_WATERMARK_OVERLAY_H
