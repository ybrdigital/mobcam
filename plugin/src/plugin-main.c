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
#include <obs-module.h>
#include "mobcam-source.h"
#include "net/discovery_listener.h"
#include "ui/web_control_server.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("mobcam", "en-US")

MODULE_EXPORT const char *obs_module_description(void) {
    return "MobCam Android Webcam Source (DroidCam-class, watermark-free)";
}

MODULE_EXPORT const char *obs_module_name(void) {
    return "MobCam";
}

bool obs_module_load(void) {
    mobcam_discovery_init();
    mobcam_web_server_start(4752);
    obs_register_source(&mobcam_source_info);
    blog(LOG_INFO, "[MobCam] Plugin loaded successfully with Auto-Discovery & Quick Controls Web Server (version 1.1.0)");
    return true;
}

void obs_module_unload(void) {
    mobcam_web_server_stop();
    mobcam_discovery_cleanup();
    blog(LOG_INFO, "[MobCam] Plugin unloaded");
}

