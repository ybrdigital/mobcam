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
#ifndef MOBCAM_CONTROL_CLIENT_H
#define MOBCAM_CONTROL_CLIENT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mobcam_config {
    char resolution[32]; // e.g. "1920x1080"
    int width;
    int height;
    int fps;             // 15, 24, 30, 60
    int bitrate_kbps;    // e.g. 8000
    bool audio_enabled;
    char camera[16];     // "back" or "front"
    char codec[16];      // "h264" or "hevc"
} mobcam_config_t;

typedef struct mobcam_control_client mobcam_control_client_t;

typedef void (*mobcam_state_callback_t)(void *user_data, const char *camera, bool torch, int battery_pct, bool is_pro, int rotation, float zoom, int exposure, const char *wb);
typedef void (*mobcam_stream_info_callback_t)(void *user_data, const char *res, int fps, int bitrate_kbps, const char *latency_mode, bool audio, const char *codec);

mobcam_control_client_t *mobcam_control_create(const char *host, int port, const char *pin,
                                               mobcam_state_callback_t state_cb,
                                               mobcam_stream_info_callback_t stream_info_cb,
                                               void *user_data);

bool mobcam_control_connect_and_pair(mobcam_control_client_t *client, int timeout_ms);

bool mobcam_control_send_config(mobcam_control_client_t *client, const mobcam_config_t *config,
                                mobcam_config_t *out_negotiated, int timeout_ms);

void mobcam_control_poll_heartbeat(mobcam_control_client_t *client);
void mobcam_control_poll_events(mobcam_control_client_t *client);

bool mobcam_control_is_connected(mobcam_control_client_t *client);

void mobcam_control_disconnect(mobcam_control_client_t *client);

bool mobcam_control_send_camera_control(mobcam_control_client_t *client, float zoom, bool torch, int exposure, const char *camera, const char *wb);

void mobcam_control_destroy(mobcam_control_client_t *client);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_CONTROL_CLIENT_H
