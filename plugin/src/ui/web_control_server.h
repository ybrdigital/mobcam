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
#ifndef MOBCAM_WEB_CONTROL_SERVER_H
#define MOBCAM_WEB_CONTROL_SERVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mobcam_quick_source_info {
    int id;
    char name[128];
    char host[64];
    bool connected;
    float zoom;
    float max_zoom;
    bool torch;
    bool has_torch;
    int exposure;
    char camera[16];
    char wb[32];
} mobcam_quick_source_info_t;

typedef struct mobcam_multi_status {
    int count;
    mobcam_quick_source_info_t sources[16];
} mobcam_multi_status_t;

typedef void (*mobcam_web_apply_ctrl_cb)(int source_id, float zoom, int torch, int exposure, const char *camera, const char *wb);
typedef void (*mobcam_web_query_multi_status_cb)(mobcam_multi_status_t *out_status);

bool mobcam_web_server_start(int port);
void mobcam_web_server_stop(void);
void mobcam_web_server_set_callbacks(mobcam_web_query_multi_status_cb query_cb, mobcam_web_apply_ctrl_cb apply_cb);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_WEB_CONTROL_SERVER_H
