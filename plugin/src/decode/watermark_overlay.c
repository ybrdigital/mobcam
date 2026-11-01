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
#include "watermark_overlay.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Standard 8x8 monospace bitmap font for ASCII 32 (' ') to 126 ('~')
static const uint8_t s_font8x8[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // 33 '!'
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, // 34 '"'
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, // 35 '#'
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, // 36 '$'
    {0x00,0x63,0x66,0x0C,0x18,0x33,0x63,0x00}, // 37 '%'
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, // 38 '&'
    {0x30,0x30,0x10,0x20,0x00,0x00,0x00,0x00}, // 39 '''
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // 40 '('
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // 41 ')'
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // 42 '*'
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // 43 '+'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // 44 ','
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // 45 '-'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // 46 '.'
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, // 47 '/'
    {0x7C,0xC6,0xCE,0xD6,0xE6,0xC6,0x7C,0x00}, // 48 '0'
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // 49 '1'
    {0x7C,0xC6,0x06,0x1C,0x30,0x66,0xFE,0x00}, // 50 '2'
    {0x7C,0xC6,0x06,0x3C,0x06,0xC6,0x7C,0x00}, // 51 '3'
    {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00}, // 52 '4'
    {0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00}, // 53 '5'
    {0x78,0x0C,0xC0,0xFC,0xC6,0xC6,0x7C,0x00}, // 54 '6'
    {0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00}, // 55 '7'
    {0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00}, // 56 '8'
    {0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00}, // 57 '9'
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // 58 ':'
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30}, // 59 ';'
    {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00}, // 60 '<'
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // 61 '='
    {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00}, // 62 '>'
    {0x7C,0xC6,0x0C,0x18,0x18,0x00,0x18,0x00}, // 63 '?'
    {0x7C,0xC6,0xDE,0xDE,0xDC,0xC0,0x7C,0x00}, // 64 '@'
    {0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00}, // 65 'A'
    {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00}, // 66 'B'
    {0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00}, // 67 'C'
    {0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00}, // 68 'D'
    {0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00}, // 69 'E'
    {0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00}, // 70 'F'
    {0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00}, // 71 'G'
    {0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00}, // 72 'H'
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00}, // 73 'I'
    {0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00}, // 74 'J'
    {0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00}, // 75 'K'
    {0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00}, // 76 'L'
    {0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00}, // 77 'M'
    {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00}, // 78 'N'
    {0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00}, // 79 'O'
    {0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00}, // 80 'P'
    {0x7C,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x06}, // 81 'Q'
    {0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0x00}, // 82 'R'
    {0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00}, // 83 'S'
    {0x7E,0x5A,0x18,0x18,0x18,0x18,0x3C,0x00}, // 84 'T'
    {0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00}, // 85 'U'
    {0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00}, // 86 'V'
    {0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0xC6,0x00}, // 87 'W'
    {0xC6,0x6C,0x38,0x38,0x6C,0xC6,0xC6,0x00}, // 88 'X'
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x3C,0x00}, // 89 'Y'
    {0xFE,0xC6,0x8C,0x18,0x32,0x66,0xFE,0x00}, // 90 'Z'
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // 91 '['
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, // 92 '\'
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // 93 ']'
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00}, // 94 '^'
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, // 95 '_'
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, // 96 '`'
    {0x00,0x00,0x78,0x0C,0x7C,0xCC,0x76,0x00}, // 97 'a'
    {0xE0,0x60,0x7C,0x66,0x66,0x66,0xDC,0x00}, // 98 'b'
    {0x00,0x00,0x7C,0xC6,0xC0,0xC6,0x7C,0x00}, // 99 'c'
    {0x1C,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00}, // 100 'd'
    {0x00,0x00,0x7C,0xC6,0xFE,0xC0,0x7C,0x00}, // 101 'e'
    {0x1C,0x36,0x30,0x78,0x30,0x30,0x78,0x00}, // 102 'f'
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0xF8}, // 103 'g'
    {0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00}, // 104 'h'
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, // 105 'i'
    {0x06,0x00,0x0E,0x06,0x06,0x66,0x66,0x3C}, // 106 'j'
    {0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00}, // 107 'k'
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 108 'l'
    {0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xD6,0x00}, // 109 'm'
    {0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x00}, // 110 'n'
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00}, // 111 'o'
    {0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0}, // 112 'p'
    {0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E}, // 113 'q'
    {0x00,0x00,0xDC,0x76,0x60,0x60,0xF0,0x00}, // 114 'r'
    {0x00,0x00,0x7C,0xC0,0x7C,0x06,0xF8,0x00}, // 115 's'
    {0x10,0x30,0x7C,0x30,0x30,0x34,0x18,0x00}, // 116 't'
    {0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00}, // 117 'u'
    {0x00,0x00,0xC6,0xC6,0xC6,0x6C,0x38,0x00}, // 118 'v'
    {0x00,0x00,0xC6,0xD6,0xD6,0xFE,0x6C,0x00}, // 119 'w'
    {0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00}, // 120 'x'
    {0x00,0x00,0xC6,0xC6,0xCE,0x76,0x06,0xF8}, // 121 'y'
    {0x00,0x00,0xFE,0xCC,0x38,0x66,0xFE,0x00}, // 122 'z'
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // 123 '{'
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, // 124 '|'
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // 125 '}'
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00}  // 126 '~'
};

static inline void blend_yuv_pixel(
    struct obs_source_frame *frame,
    int px, int py,
    uint8_t target_y, uint8_t target_u, uint8_t target_v,
    float alpha
) {
    if (px < 0 || px >= (int)frame->width || py < 0 || py >= (int)frame->height) return;

    // Y plane (luma)
    uint8_t *y_ptr = frame->data[0] + py * frame->linesize[0] + px;
    *y_ptr = (uint8_t)((*y_ptr) * (1.0f - alpha) + target_y * alpha);

    // Chroma planes (subsampled 2x2): ONLY blend once per 2x2 block to prevent pixel glitching
    if ((px & 1) == 0 && (py & 1) == 0) {
        int uv_x = px / 2;
        int uv_y = py / 2;

        if (frame->format == VIDEO_FORMAT_NV12) {
            uint8_t *uv_ptr = frame->data[1] + uv_y * frame->linesize[1] + (uv_x * 2);
            uv_ptr[0] = (uint8_t)(uv_ptr[0] * (1.0f - alpha) + target_u * alpha);
            uv_ptr[1] = (uint8_t)(uv_ptr[1] * (1.0f - alpha) + target_v * alpha);
        } else if (frame->format == VIDEO_FORMAT_I420) {
            uint8_t *u_ptr = frame->data[1] + uv_y * frame->linesize[1] + uv_x;
            uint8_t *v_ptr = frame->data[2] + uv_y * frame->linesize[2] + uv_x;
            *u_ptr = (uint8_t)((*u_ptr) * (1.0f - alpha) + target_u * alpha);
            *v_ptr = (uint8_t)((*v_ptr) * (1.0f - alpha) + target_v * alpha);
        }
    }
}

static void draw_glyph(
    struct obs_source_frame *frame,
    char c,
    int start_x, int start_y,
    int scale,
    uint8_t target_y, uint8_t target_u, uint8_t target_v,
    float alpha
) {
    if (c < 32 || c > 126) c = ' ';
    const uint8_t *rows = s_font8x8[c - 32];

    for (int r = 0; r < 8; r++) {
        uint8_t row_bits = rows[r];
        if (!row_bits) continue;
        for (int col = 0; col < 8; col++) {
            if ((row_bits >> (7 - col)) & 1) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        blend_yuv_pixel(
                            frame,
                            start_x + col * scale + sx,
                            start_y + r * scale + sy,
                            target_y, target_u, target_v,
                            alpha
                        );
                    }
                }
            }
        }
    }
}

static void draw_text(
    struct obs_source_frame *frame,
    const char *text,
    int start_x, int start_y,
    int scale,
    uint8_t y, uint8_t u, uint8_t v,
    float alpha
) {
    size_t len = strlen(text);
    int char_spacing = scale * 8;

    for (size_t i = 0; i < len; i++) {
        draw_glyph(frame, text[i], start_x + (int)i * char_spacing, start_y, scale, y, u, v, alpha);
    }
}

void mobcam_draw_free_watermark(struct obs_source_frame *obs_frame, float opacity, mobcam_watermark_backup_t *backup) {
    if (!obs_frame || !obs_frame->data[0]) return;

    int w = (int)obs_frame->width;
    int h = (int)obs_frame->height;
    if (w < 320 || h < 240) return;

    // Fixed opacity 0.7 as requested
    float alpha = (opacity > 0.0f) ? opacity : 0.7f;
    if (alpha > 0.7f) alpha = 0.7f;

    // Subtitle ("Streaming From") and Main Brand ("MobCam")
    const char *sub_text = "Streaming From";
    const char *main_text = "MobCam";

    int sub_len = (int)strlen(sub_text);
    int main_len = (int)strlen(main_text);

    // Font scales: Subtitle in small size, main text larger
    int sub_scale = (w >= 3840) ? 3 : (w >= 1280) ? 2 : 1;
    int main_scale = (w >= 3840) ? 6 : (w >= 1280) ? 4 : 2;

    int sub_w = sub_len * (sub_scale * 8);
    int sub_h = sub_scale * 8;

    int main_w = main_len * (main_scale * 8);
    int main_h = main_scale * 8;

    int gap = sub_scale * 4;
    int total_h = sub_h + gap + main_h;

    // Center both lines horizontally (aligned to even pixel boundaries)
    int sub_x = ((w - sub_w) / 2) & ~1;
    int main_x = ((w - main_w) / 2) & ~1;

    // Center watermark block vertically
    int start_y = ((h - total_h) / 2) & ~1;
    int sub_y = start_y;
    int main_y = (sub_y + sub_h + gap) & ~1;

    // Bounding box encompassing both lines for pristine background backup
    int box_x = (sub_x < main_x ? sub_x : main_x) & ~1;
    int right_x = (sub_x + sub_w > main_x + main_w ? sub_x + sub_w : main_x + main_w);
    int box_w = ((right_x - box_x) + 1) & ~1;
    int box_h = (total_h + 1) & ~1;
    int box_y = start_y & ~1;

    // Ensure within frame bounds
    if (box_x < 0) box_x = 0;
    if (box_y < 0) box_y = 0;
    if (box_x + box_w > w) box_w = (w - box_x) & ~1;
    if (box_y + box_h > h) box_h = (h - box_y) & ~1;

    // 1. Back up original pristine pixels so FFmpeg's DPB reference buffer is never corrupted
    if (backup) {
        size_t needed = (size_t)(box_w * box_h * 2 + 1024);
        if (backup->capacity < needed) {
            free(backup->y_buf);
            backup->y_buf = (uint8_t *)malloc(needed);
            backup->capacity = needed;
        }

        if (backup->y_buf) {
            backup->x = box_x;
            backup->y = box_y;
            backup->w = box_w;
            backup->h = box_h;
            backup->format = obs_frame->format;
            backup->u_buf = backup->y_buf + (box_w * box_h);
            backup->v_buf = backup->u_buf + ((box_w / 2) * (box_h / 2));
            backup->active = true;

            // Copy Y plane
            for (int r = 0; r < box_h; r++) {
                const uint8_t *src = obs_frame->data[0] + (box_y + r) * obs_frame->linesize[0] + box_x;
                uint8_t *dst = backup->y_buf + r * box_w;
                memcpy(dst, src, box_w);
            }

            // Copy Chroma plane(s)
            int uv_h = box_h / 2;
            int uv_y_start = box_y / 2;

            if (obs_frame->format == VIDEO_FORMAT_NV12) {
                int uv_row_bytes = box_w;
                for (int r = 0; r < uv_h; r++) {
                    const uint8_t *src = obs_frame->data[1] + (uv_y_start + r) * obs_frame->linesize[1] + box_x;
                    uint8_t *dst = backup->u_buf + r * uv_row_bytes;
                    memcpy(dst, src, uv_row_bytes);
                }
            } else if (obs_frame->format == VIDEO_FORMAT_I420) {
                int uv_w = box_w / 2;
                int uv_x_start = box_x / 2;
                for (int r = 0; r < uv_h; r++) {
                    const uint8_t *src_u = obs_frame->data[1] + (uv_y_start + r) * obs_frame->linesize[1] + uv_x_start;
                    const uint8_t *src_v = obs_frame->data[2] + (uv_y_start + r) * obs_frame->linesize[2] + uv_x_start;
                    uint8_t *dst_u = backup->u_buf + r * uv_w;
                    uint8_t *dst_v = backup->v_buf + r * uv_w;
                    memcpy(dst_u, src_u, uv_w);
                    memcpy(dst_v, src_v, uv_w);
                }
            }
        }
    }

    // 2. Draw clean watermark
    // Subtitle: "Streaming From" in small size (Y=230, U=128, V=128)
    draw_text(obs_frame, sub_text, sub_x, sub_y, sub_scale, 230, 128, 128, alpha * 0.9f);

    // Main text: "MobCam" (Y=240, U=128, V=128)
    draw_text(obs_frame, main_text, main_x, main_y, main_scale, 240, 128, 128, alpha);
}

void mobcam_restore_watermark_background(struct obs_source_frame *obs_frame, mobcam_watermark_backup_t *backup) {
    if (!obs_frame || !backup || !backup->active || !backup->y_buf) return;

    int box_w = backup->w;
    int box_h = backup->h;
    int start_x = backup->x;
    int start_y = backup->y;

    // Restore Y plane
    for (int r = 0; r < box_h; r++) {
        uint8_t *dst = obs_frame->data[0] + (start_y + r) * obs_frame->linesize[0] + start_x;
        const uint8_t *src = backup->y_buf + r * box_w;
        memcpy(dst, src, box_w);
    }

    // Restore Chroma plane(s)
    int uv_h = box_h / 2;
    int uv_y_start = start_y / 2;

    if (obs_frame->format == VIDEO_FORMAT_NV12) {
        int uv_row_bytes = box_w;
        for (int r = 0; r < uv_h; r++) {
            uint8_t *dst = obs_frame->data[1] + (uv_y_start + r) * obs_frame->linesize[1] + start_x;
            const uint8_t *src = backup->u_buf + r * uv_row_bytes;
            memcpy(dst, src, uv_row_bytes);
        }
    } else if (obs_frame->format == VIDEO_FORMAT_I420) {
        int uv_w = box_w / 2;
        int uv_x_start = start_x / 2;
        for (int r = 0; r < uv_h; r++) {
            uint8_t *dst_u = obs_frame->data[1] + (uv_y_start + r) * obs_frame->linesize[1] + uv_x_start;
            uint8_t *dst_v = obs_frame->data[2] + (uv_y_start + r) * obs_frame->linesize[2] + uv_x_start;
            const uint8_t *src_u = backup->u_buf + r * uv_w;
            const uint8_t *src_v = backup->v_buf + r * uv_w;
            memcpy(dst_u, src_u, uv_w);
            memcpy(dst_v, src_v, uv_w);
        }
    }

    backup->active = false;
}

void mobcam_watermark_backup_free(mobcam_watermark_backup_t *backup) {
    if (!backup) return;
    if (backup->y_buf) {
        free(backup->y_buf);
        backup->y_buf = NULL;
        backup->u_buf = NULL;
        backup->v_buf = NULL;
    }
    backup->capacity = 0;
    backup->active = false;
}
