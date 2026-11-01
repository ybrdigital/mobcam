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
#include "mobcam-properties.h"
#include "net/discovery_listener.h"
#include <obs-module.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

static bool on_open_controls_clicked(obs_properties_t *props, obs_property_t *property, void *data) {
    (void)props; (void)property; (void)data;
#ifdef _WIN32
    ShellExecuteA(NULL, "open", "http://localhost:4752/", NULL, NULL, SW_SHOWNORMAL);
#endif
    return true;
}

static bool on_device_selected(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
    (void)property;
    const char *selection = obs_data_get_string(settings, "device_selection");
    bool is_manual = (strcmp(selection, "manual") == 0);

    obs_property_t *host_prop = obs_properties_get(props, "host");
    obs_property_t *port_prop = obs_properties_get(props, "port");
    if (host_prop) obs_property_set_visible(host_prop, is_manual);
    if (port_prop) obs_property_set_visible(port_prop, is_manual);

    if (!is_manual) {
        char host[64] = {0};
        int port = 4747;
        bool is_usb = false;
        if (mobcam_discovery_get_preferred_ip(selection, host, sizeof(host), &port, &is_usb)) {
            obs_data_set_string(settings, "host", host);
            obs_data_set_int(settings, "port", port);
            obs_data_set_bool(settings, "usb_mode", is_usb);
        }
    }

    return true;
}

static bool on_refresh_devices_clicked(obs_properties_t *props, obs_property_t *property, void *data) {
    (void)property; (void)data;

    obs_property_t *dev_prop = obs_properties_get(props, "device_selection");
    if (dev_prop) {
        obs_property_list_clear(dev_prop);
        obs_property_list_add_string(dev_prop, "[Auto-Detect (Seamless USB/Wi-Fi)]", "auto");

        mobcam_discovered_device_t devices[16];
        size_t count = mobcam_discovery_get_devices(devices, 16);
        for (size_t i = 0; i < count; i++) {
            char label[256];
            const char *transport_str = (devices[i].usb_ip[0] != '\0') ? "USB Wired" : "Wi-Fi";
            const char *ip_str = (devices[i].usb_ip[0] != '\0') ? devices[i].usb_ip : devices[i].wifi_ip;
            snprintf(label, sizeof(label), "%s [%s: %s]",
                     devices[i].name, transport_str, ip_str);
            obs_property_list_add_string(dev_prop, label, devices[i].id);
        }

        obs_property_list_add_string(dev_prop, "[Manual IP Entry]", "manual");
    }

    return true;
}

void mobcam_get_defaults(obs_data_t *settings) {
    obs_data_set_default_string(settings, "device_selection", "auto");
    obs_data_set_default_string(settings, "host", "");
    obs_data_set_default_int(settings, "port", 4747);
    obs_data_set_default_bool(settings, "usb_mode", false);

    // Read-only stream information (populated live from mobile app)
    obs_data_set_default_string(settings, "res_display", "Waiting for app...");
    obs_data_set_default_string(settings, "fps_display", "Waiting for app...");
    obs_data_set_default_string(settings, "bitrate_display", "Waiting for app...");
    obs_data_set_default_string(settings, "latency_display", "Waiting for app...");
    obs_data_set_default_string(settings, "audio_display", "Waiting for app...");
    obs_data_set_default_string(settings, "codec_display", "Waiting for app...");

    // Internal camera settings (managed via MobCam Controls dock menu)
    obs_data_set_default_double(settings, "zoom", 1.0);
    obs_data_set_default_bool(settings, "torch", false);
    obs_data_set_default_int(settings, "exposure", 0);
    obs_data_set_default_string(settings, "wb", "auto");
    obs_data_set_default_string(settings, "camera", "back");
}

obs_properties_t *mobcam_get_properties(void *data) {
    (void)data;
    obs_properties_t *props = obs_properties_create();

    // 1. Device Discovery & Selection (The ONLY editable configuration)
    obs_property_t *dev_prop = obs_properties_add_list(props, "device_selection",
                                                       obs_module_text("Select Camera Device"),
                                                       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(dev_prop, "[Auto-Detect (Seamless USB/Wi-Fi)]", "auto");

    mobcam_discovered_device_t devices[16];
    size_t count = mobcam_discovery_get_devices(devices, 16);
    for (size_t i = 0; i < count; i++) {
        char label[256];
        const char *transport_str = (devices[i].usb_ip[0] != '\0') ? "USB Wired" : "Wi-Fi";
        const char *ip_str = (devices[i].usb_ip[0] != '\0') ? devices[i].usb_ip : devices[i].wifi_ip;
        snprintf(label, sizeof(label), "%s [%s: %s]",
                 devices[i].name, transport_str, ip_str);
        obs_property_list_add_string(dev_prop, label, devices[i].id);
    }
    obs_property_list_add_string(dev_prop, "[Manual IP Entry]", "manual");
    obs_property_set_modified_callback(dev_prop, on_device_selected);

    obs_properties_add_button(props, "refresh_btn", obs_module_text("Scan / Refresh Devices"), on_refresh_devices_clicked);
    obs_properties_add_button(props, "open_controls_btn", obs_module_text("Open MobCam Controls (Web Panel)"), on_open_controls_clicked);

    // Manual Host/Port inputs (hidden by default unless "Manual" is selected)
    obs_property_t *host_prop = obs_properties_add_text(props, "host", obs_module_text("Manual Host / IP"), OBS_TEXT_DEFAULT);
    obs_property_t *port_prop = obs_properties_add_int(props, "port", obs_module_text("Manual Port"), 1, 65535, 1);
    obs_property_set_visible(host_prop, false);
    obs_property_set_visible(port_prop, false);

    // 2. Stream Details (Strictly Read-Only - Controlled via Mobile App)
    obs_property_t *p_res = obs_properties_add_text(props, "res_display",
                                                   obs_module_text("Resolution (Controlled via App)"),
                                                   OBS_TEXT_DEFAULT);
    obs_property_set_enabled(p_res, false);

    obs_property_t *p_fps = obs_properties_add_text(props, "fps_display",
                                                   obs_module_text("Framerate (Controlled via App)"),
                                                   OBS_TEXT_DEFAULT);
    obs_property_set_enabled(p_fps, false);

    obs_property_t *p_bitrate = obs_properties_add_text(props, "bitrate_display",
                                                       obs_module_text("Bitrate (Controlled via App)"),
                                                       OBS_TEXT_DEFAULT);
    obs_property_set_enabled(p_bitrate, false);

    obs_property_t *p_lat = obs_properties_add_text(props, "latency_display",
                                                   obs_module_text("Latency Mode (Controlled via App)"),
                                                   OBS_TEXT_DEFAULT);
    obs_property_set_enabled(p_lat, false);

    obs_property_t *p_audio = obs_properties_add_text(props, "audio_display",
                                                     obs_module_text("Audio Stream (Controlled via App)"),
                                                     OBS_TEXT_DEFAULT);
    obs_property_set_enabled(p_audio, false);

    obs_property_t *p_codec = obs_properties_add_text(props, "codec_display",
                                                     obs_module_text("Video Format (Controlled via App)"),
                                                     OBS_TEXT_DEFAULT);
    obs_property_set_enabled(p_codec, false);

    return props;
}
