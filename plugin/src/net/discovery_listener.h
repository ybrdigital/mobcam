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
#ifndef MOBCAM_DISCOVERY_LISTENER_H
#define MOBCAM_DISCOVERY_LISTENER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mobcam_discovered_device {
    char id[64];
    char name[128];
    char usb_ip[64];
    uint64_t last_seen_usb_ms;
    char wifi_ip[64];
    uint64_t last_seen_wifi_ms;
    int port;
} mobcam_discovered_device_t;

void mobcam_discovery_init(void);
void mobcam_discovery_cleanup(void);
size_t mobcam_discovery_get_devices(mobcam_discovered_device_t *out_devices, size_t max_count);
bool mobcam_discovery_get_preferred_ip(const char *device_id, char *out_ip, size_t max_ip_len, int *out_port, bool *out_is_usb);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_DISCOVERY_LISTENER_H
