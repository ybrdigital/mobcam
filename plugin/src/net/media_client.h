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
#ifndef MOBCAM_MEDIA_CLIENT_H
#define MOBCAM_MEDIA_CLIENT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mobcam_frame_callback_t)(void *user_data, const uint8_t *data, size_t size, int64_t pts_us);

typedef struct mobcam_media_client mobcam_media_client_t;

mobcam_media_client_t *mobcam_media_client_create(const char *host, int port,
                                                  mobcam_frame_callback_t frame_cb,
                                                  void *user_data);

bool mobcam_media_client_start(mobcam_media_client_t *client);

void mobcam_media_client_stop(mobcam_media_client_t *client);

bool mobcam_media_client_is_running(mobcam_media_client_t *client);

void mobcam_media_client_destroy(mobcam_media_client_t *client);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_MEDIA_CLIENT_H
