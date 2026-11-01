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
#include "mobcam-source.h"
#include "net/control_client.h"
#include "net/media_client.h"
#include "decode/h264_decoder.h"
#include "decode/aac_decoder.h"
#include "decode/watermark_overlay.h"
#include "ui/mobcam-properties.h"
#include "net/discovery_listener.h"
#include "ui/web_control_server.h"

#include <obs-module.h>
#include <util/platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define THREAD_HANDLE HANDLE
#define THREAD_FUNC DWORD WINAPI
#define SLEEP_MS(ms) Sleep(ms)
#define MUTEX_HANDLE CRITICAL_SECTION
#define MUTEX_INIT(m) InitializeCriticalSection(m)
#define MUTEX_LOCK(m) EnterCriticalSection(m)
#define MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#define MUTEX_DESTROY(m) DeleteCriticalSection(m)
#else
#include <pthread.h>
#include <unistd.h>
#define THREAD_HANDLE pthread_t
#define THREAD_FUNC void *
#define SLEEP_MS(ms) usleep((ms) * 1000)
#define MUTEX_HANDLE pthread_mutex_t
#define MUTEX_INIT(m) pthread_mutex_init(m, NULL)
#define MUTEX_LOCK(m) pthread_mutex_lock(m)
#define MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#define MUTEX_DESTROY(m) pthread_mutex_destroy(m)
#endif

typedef struct mobcam_source_data {
    obs_source_t *source;

    char device_selection[64];
    char host[128];
    int port;
    char pin[16];
    mobcam_config_t config;
    bool usb_mode;
    bool ultra_low_latency;
    bool in_internal_update;

    float zoom;
    bool torch;
    int exposure;
    char wb[32];

    volatile bool running;
    THREAD_HANDLE worker_thread;

    mobcam_control_client_t *control_client;
    mobcam_media_client_t *video_client;
    mobcam_media_client_t *audio_client;

    mobcam_h264_decoder_t *video_decoder;
    mobcam_aac_decoder_t *audio_decoder;
    MUTEX_HANDLE video_mutex;
    MUTEX_HANDLE audio_mutex;

    // Hardware-synchronized clock tracking for perfect A/V sync
    MUTEX_HANDLE clock_mutex;
    bool clock_synced;
    int64_t base_device_pts_us;
    uint64_t base_obs_time_ns;
    int64_t clock_offset_ns;

    uint32_t video_frames_count;
    uint32_t audio_frames_count;
    uint64_t last_audio_ts_ns;

    bool is_pro;
    int device_rotation;
    mobcam_watermark_backup_t watermark_backup;
} mobcam_source_data_t;

#define MAX_MOBCAM_SOURCES 16
static mobcam_source_data_t *s_sources[MAX_MOBCAM_SOURCES] = {0};
static size_t s_sources_count = 0;
static MUTEX_HANDLE s_sources_mutex;
static bool s_sources_mutex_init = false;

static void init_sources_registry(void) {
    if (!s_sources_mutex_init) {
        MUTEX_INIT(&s_sources_mutex);
        s_sources_mutex_init = true;
    }
}

static void mobcam_source_query_status(mobcam_multi_status_t *out_status) {
    if (!out_status) return;
    memset(out_status, 0, sizeof(*out_status));
    init_sources_registry();

    MUTEX_LOCK(&s_sources_mutex);
    out_status->count = (int)s_sources_count;
    for (size_t i = 0; i < s_sources_count && i < 16; i++) {
        mobcam_source_data_t *s = s_sources[i];
        mobcam_quick_source_info_t *item = &out_status->sources[i];
        item->id = (int)i;
        const char *src_name = s->source ? obs_source_get_name(s->source) : NULL;
        if (src_name && strlen(src_name) > 0) {
            snprintf(item->name, sizeof(item->name), "%s", src_name);
        } else {
            snprintf(item->name, sizeof(item->name), "MobCam %d", (int)i + 1);
        }
        strncpy(item->host, s->host, sizeof(item->host) - 1);
        item->connected = (s->control_client && mobcam_control_is_connected(s->control_client));
        item->zoom = s->zoom;
        item->max_zoom = 8.0f;
        item->torch = s->torch;
        item->has_torch = true;
        item->exposure = s->exposure;
        strncpy(item->camera, s->config.camera, sizeof(item->camera) - 1);
        strncpy(item->wb, s->wb, sizeof(item->wb) - 1);
    }
    MUTEX_UNLOCK(&s_sources_mutex);
}

static void mobcam_source_apply_ctrl(int source_id, float zoom, int torch, int exposure, const char *camera, const char *wb) {
    init_sources_registry();
    mobcam_source_data_t *target = NULL;

    MUTEX_LOCK(&s_sources_mutex);
    if (source_id >= 0 && (size_t)source_id < s_sources_count) {
        target = s_sources[source_id];
    } else if (s_sources_count > 0) {
        target = s_sources[0];
    }
    MUTEX_UNLOCK(&s_sources_mutex);

    if (!target) return;

    if (zoom >= 1.0f) {
        target->zoom = zoom;
    }
    if (torch >= 0) {
        target->torch = (torch != 0);
    }
    if (exposure >= -12 && exposure <= 12) {
        target->exposure = exposure;
    }
    if (camera && strlen(camera) > 0) {
        strncpy(target->config.camera, camera, sizeof(target->config.camera) - 1);
    }
    if (wb && strlen(wb) > 0) {
        strncpy(target->wb, wb, sizeof(target->wb) - 1);
    }

    if (target->source) {
        obs_data_t *settings = obs_source_get_settings(target->source);
        if (settings) {
            if (zoom >= 1.0f) obs_data_set_double(settings, "zoom", (double)target->zoom);
            if (torch >= 0) obs_data_set_bool(settings, "torch", target->torch);
            if (exposure >= -12 && exposure <= 12) obs_data_set_int(settings, "exposure", target->exposure);
            if (camera && strlen(camera) > 0) obs_data_set_string(settings, "camera", target->config.camera);
            if (wb && strlen(wb) > 0) obs_data_set_string(settings, "wb", target->wb);
            obs_data_release(settings);
        }
    }

    if (target->control_client && mobcam_control_is_connected(target->control_client)) {
        mobcam_control_send_camera_control(
            target->control_client,
            target->zoom,
            target->torch,
            target->exposure,
            target->config.camera,
            target->wb
        );
    }
}

static const char *mobcam_get_name(void *unused) {
    (void)unused;
    return obs_module_text("MobCam");
}

static uint64_t sync_device_timestamp(mobcam_source_data_t *s, int64_t pts_us) {
    if (pts_us <= 0) {
        return os_gettime_ns();
    }

    MUTEX_LOCK(&s->clock_mutex);
    uint64_t cur_obs_ns = os_gettime_ns();
    int64_t device_ns = pts_us * 1000LL;

    if (!s->clock_synced) {
        s->base_device_pts_us = pts_us;
        s->base_obs_time_ns = cur_obs_ns;
        s->clock_offset_ns = (int64_t)cur_obs_ns - device_ns;
        s->clock_synced = true;
        MUTEX_UNLOCK(&s->clock_mutex);
        return cur_obs_ns;
    }

    int64_t target_obs_ns = device_ns + s->clock_offset_ns;

    // Detect reset / disconnection / discontinuity (> 500ms jump)
    int64_t diff = (int64_t)cur_obs_ns - target_obs_ns;
    if (diff < -500000000LL || diff > 500000000LL) {
        s->clock_offset_ns = (int64_t)cur_obs_ns - device_ns;
        target_obs_ns = (int64_t)cur_obs_ns;
    } else {
        // Continuous smooth drift compensation:
        // Keeps OBS presentation time closely aligned with device camera hardware cadence,
        // while tracking network delivery smoothly without judder or frame drops.
        int64_t target_cushion_ns = s->ultra_low_latency ? 25000000LL : 50000000LL; // 25ms cushion in ultra_low, 50ms in buffered
        int64_t drift = diff - target_cushion_ns;
        if (drift > 50000000LL) {
            // Packets delayed by Wi-Fi burst, gently catch up
            s->clock_offset_ns += drift / 8;
        } else if (drift < -15000000LL) {
            // Running ahead of real time, slow down slightly
            s->clock_offset_ns += drift / 16;
        }
        target_obs_ns = device_ns + s->clock_offset_ns;
    }

    MUTEX_UNLOCK(&s->clock_mutex);
    return (uint64_t)target_obs_ns;
}

static void on_control_state_changed(void *user_data, const char *camera, bool torch, int battery_pct, bool is_pro, int rotation, float zoom, int exposure, const char *wb) {
    (void)camera; (void)torch; (void)battery_pct; (void)zoom; (void)exposure; (void)wb;
    mobcam_source_data_t *s = (mobcam_source_data_t *)user_data;
    if (!s) return;
    s->is_pro = is_pro;
    if (s->device_rotation != rotation) {
        s->device_rotation = rotation;
    }
}

static void on_video_decoded(void *user_data, const mobcam_video_frame_t *frame);

static void on_stream_info_received(void *user_data, const char *res, int fps, int bitrate_kbps, const char *latency_mode, bool audio, const char *codec) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)user_data;
    if (!s || !s->running) return;

    if (res && strlen(res) > 0) {
        strncpy(s->config.resolution, res, sizeof(s->config.resolution) - 1);
        sscanf(s->config.resolution, "%dx%d", &s->config.width, &s->config.height);
    }
    if (fps > 0) s->config.fps = fps;
    if (bitrate_kbps > 0) s->config.bitrate_kbps = bitrate_kbps;
    s->config.audio_enabled = audio;
    if (codec && strlen(codec) > 0) {
        bool codec_changed = (strlen(s->config.codec) > 0 && strcmp(s->config.codec, codec) != 0);
        strncpy(s->config.codec, codec, sizeof(s->config.codec) - 1);
        if (codec_changed && s->video_decoder) {
            MUTEX_LOCK(&s->video_mutex);
            mobcam_h264_decoder_destroy(s->video_decoder);
            s->video_decoder = mobcam_h264_decoder_create(s->config.codec, on_video_decoded, s);
            MUTEX_UNLOCK(&s->video_mutex);
            blog(LOG_INFO, "[MobCam] Video decoder recreated for newly negotiated codec: %s", s->config.codec);
        }
    }

    s->ultra_low_latency = (!latency_mode || strcmp(latency_mode, "buffered") != 0);
    if (s->source) {
        obs_source_set_async_unbuffered(s->source, s->ultra_low_latency);

        obs_data_t *settings = obs_source_get_settings(s->source);
        if (settings) {
            char res_buf[64], fps_buf[64], bitrate_buf[64], lat_buf[64], audio_buf[64], codec_buf[64];
            snprintf(res_buf, sizeof(res_buf), "%s (from App)", s->config.resolution);
            snprintf(fps_buf, sizeof(fps_buf), "%d FPS (from App)", s->config.fps);
            snprintf(bitrate_buf, sizeof(bitrate_buf), "%d kbps (from App)", s->config.bitrate_kbps);
            snprintf(lat_buf, sizeof(lat_buf), "%s (from App)", s->ultra_low_latency ? "Ultra-Low Latency (<50ms)" : "Smooth / Buffered");
            snprintf(audio_buf, sizeof(audio_buf), "%s (from App)", audio ? "Enabled (48 kHz AAC)" : "Disabled");
            snprintf(codec_buf, sizeof(codec_buf), "%s (from App)", (strcmp(s->config.codec, "hevc") == 0 || strcmp(s->config.codec, "h265") == 0) ? "HEVC/H.265" : "AVC/H.264");

            obs_data_set_string(settings, "res_display", res_buf);
            obs_data_set_string(settings, "fps_display", fps_buf);
            obs_data_set_string(settings, "bitrate_display", bitrate_buf);
            obs_data_set_string(settings, "latency_display", lat_buf);
            obs_data_set_string(settings, "audio_display", audio_buf);
            obs_data_set_string(settings, "codec_display", codec_buf);

            obs_data_release(settings);
        }
    }

    blog(LOG_INFO, "[MobCam] Stream info from app: %s @ %d FPS, %d kbps, codec=%s, latency=%s, audio=%d",
         s->config.resolution, s->config.fps, s->config.bitrate_kbps, s->config.codec,
         s->ultra_low_latency ? "ultra_low" : "buffered", (int)audio);
}

static void on_video_decoded(void *user_data, const mobcam_video_frame_t *frame) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)user_data;
    if (!s || !s->running || !s->source || !frame) return;
    if (frame->width == 0 || frame->height == 0 || !frame->data[0]) return;

    struct obs_source_frame obs_frame;
    memset(&obs_frame, 0, sizeof(obs_frame));

    obs_frame.width = frame->width;
    obs_frame.height = frame->height;
    obs_frame.format = (frame->format == 1) ? VIDEO_FORMAT_NV12 : VIDEO_FORMAT_I420;

    video_format_get_parameters(
        VIDEO_CS_709,
        VIDEO_RANGE_PARTIAL,
        obs_frame.color_matrix,
        obs_frame.color_range_min,
        obs_frame.color_range_max
    );
    obs_frame.full_range = false;

    for (int i = 0; i < 4; i++) {
        obs_frame.data[i] = frame->data[i];
        obs_frame.linesize[i] = frame->linesize[i];
    }
    obs_frame.timestamp = sync_device_timestamp(s, frame->pts_us);

    s->video_frames_count++;
    if (s->video_frames_count == 1 || s->video_frames_count % 300 == 0) {
        blog(LOG_INFO, "[MobCam] Video frame #%u rendered: %dx%d (ts=%llu)",
             s->video_frames_count, frame->width, frame->height, (unsigned long long)obs_frame.timestamp);
    }

    // Free-tier watermark: clean white text fixed in center (0.7 opacity).
    // Original pristine pixels are backed up before drawing and restored immediately after output
    // so FFmpeg's DPB reference buffer is NEVER corrupted, completely preventing motion smearing.
    if (!s->is_pro) {
        mobcam_draw_free_watermark(&obs_frame, 0.7f, &s->watermark_backup);
    }

    obs_source_output_video(s->source, &obs_frame);

    if (!s->is_pro) {
        mobcam_restore_watermark_background(&obs_frame, &s->watermark_backup);
    }
}

static void on_audio_decoded(void *user_data, const mobcam_audio_frame_t *frame) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)user_data;
    if (!s || !s->running || !s->source || !frame) return;
    if (frame->frames == 0 || frame->samples_per_sec < 8000 || frame->samples_per_sec > 192000 ||
        frame->channels <= 0 || frame->channels > 8 || !frame->data[0]) return;

    struct obs_source_audio obs_audio;
    memset(&obs_audio, 0, sizeof(obs_audio));

    obs_audio.samples_per_sec = (uint32_t)frame->samples_per_sec;
    obs_audio.speakers = (frame->channels == 1) ? SPEAKERS_MONO : SPEAKERS_STEREO;

    switch (frame->format) {
    case 0: obs_audio.format = AUDIO_FORMAT_FLOAT_PLANAR; break;
    case 1: obs_audio.format = AUDIO_FORMAT_FLOAT; break;
    case 2: obs_audio.format = AUDIO_FORMAT_16BIT_PLANAR; break;
    case 3: default: obs_audio.format = AUDIO_FORMAT_16BIT; break;
    }

    obs_audio.frames = frame->frames;

    for (int i = 0; i < frame->channels && i < 8; i++) {
        obs_audio.data[i] = frame->data[i];
    }

    uint64_t duration_ns = (uint64_t)obs_audio.frames * 1000000000ULL / obs_audio.samples_per_sec;
    uint64_t cur_ns = os_gettime_ns();

    MUTEX_LOCK(&s->clock_mutex);
    if (s->last_audio_ts_ns == 0 ||
        (cur_ns > s->last_audio_ts_ns && (cur_ns - s->last_audio_ts_ns) > 250000000ULL) ||
        (s->last_audio_ts_ns > cur_ns && (s->last_audio_ts_ns - cur_ns) > 250000000ULL)) {
        s->last_audio_ts_ns = cur_ns - duration_ns;
    }
    s->last_audio_ts_ns += duration_ns;
    obs_audio.timestamp = s->last_audio_ts_ns;
    MUTEX_UNLOCK(&s->clock_mutex);

    s->audio_frames_count++;
    if (s->audio_frames_count == 1 || s->audio_frames_count % 300 == 0) {
        blog(LOG_INFO, "[MobCam] Audio frame #%u output: %dHz, %dch, fmt=%d, samples=%u (ts=%llu)",
             s->audio_frames_count, frame->samples_per_sec, frame->channels, obs_audio.format, frame->frames, (unsigned long long)obs_audio.timestamp);
    }

    obs_source_output_audio(s->source, &obs_audio);
}

static void on_video_raw_packet(void *user_data, const uint8_t *data, size_t size, int64_t pts_us) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)user_data;
    if (!s || !s->running || !data || size == 0) return;
    MUTEX_LOCK(&s->video_mutex);
    if (s->video_decoder && s->running) {
        mobcam_h264_decoder_decode(s->video_decoder, data, size, pts_us);
    }
    MUTEX_UNLOCK(&s->video_mutex);
}

static void on_audio_raw_packet(void *user_data, const uint8_t *data, size_t size, int64_t pts_us) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)user_data;
    if (!s || !s->running || !data || size == 0) return;
    MUTEX_LOCK(&s->audio_mutex);
    if (s->audio_decoder && s->running) {
        mobcam_aac_decoder_decode(s->audio_decoder, data, size, pts_us);
    }
    MUTEX_UNLOCK(&s->audio_mutex);
}

static THREAD_FUNC mobcam_worker_loop(void *param) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)param;

    while (s->running) {
        char target_host[128] = {0};
        int target_port = s->port;
        bool is_usb = false;

        if (strcmp(s->device_selection, "manual") != 0) {
            if (mobcam_discovery_get_preferred_ip(s->device_selection, target_host, sizeof(target_host), &target_port, &is_usb)) {
                s->usb_mode = is_usb;
                strncpy(s->host, target_host, sizeof(s->host) - 1);
                s->port = target_port;
            } else if (s->host[0] != '\0') {
                strncpy(target_host, s->host, sizeof(target_host) - 1);
            }
        } else {
            strncpy(target_host, s->host, sizeof(target_host) - 1);
        }

        if (target_host[0] == '\0') {
            SLEEP_MS(300);
            continue;
        }

        blog(LOG_INFO, "[MobCam] Connecting via %s to %s:%d...",
             is_usb ? "USB Wired" : "Wi-Fi", target_host, target_port);

        s->control_client = mobcam_control_create(target_host, target_port, "", on_control_state_changed, on_stream_info_received, s);
        if (!mobcam_control_connect_and_pair(s->control_client, 2000)) {
            mobcam_control_destroy(s->control_client);
            s->control_client = NULL;
            for (int i = 0; i < 5 && s->running; i++) {
                SLEEP_MS(100);
            }
            continue;
        }

        blog(LOG_INFO, "[MobCam] Connected! Negotiating stream...");
        mobcam_config_t negotiated;
        memset(&negotiated, 0, sizeof(negotiated));
        if (!mobcam_control_send_config(s->control_client, &s->config, &negotiated, 3000)) {
            blog(LOG_WARNING, "[MobCam] Config negotiation failed. Retrying...");
            mobcam_control_destroy(s->control_client);
            s->control_client = NULL;
            for (int i = 0; i < 5 && s->running; i++) {
                SLEEP_MS(100);
            }
            continue;
        }

        blog(LOG_INFO, "[MobCam] Stream negotiated: %s @ %dfps, %dkbps (%s)",
             negotiated.resolution, negotiated.fps, negotiated.bitrate_kbps,
             is_usb ? "USB Wired" : "Wi-Fi");

        // Send initial camera controls
        mobcam_control_send_camera_control(s->control_client, s->zoom, s->torch, s->exposure, s->config.camera, s->wb);

        // Reset clock synchronization for new session
        s->clock_synced = false;
        s->last_audio_ts_ns = 0;

        // Initialize decoders
        s->video_decoder = mobcam_h264_decoder_create(s->config.codec, on_video_decoded, s);
        if (s->config.audio_enabled) {
            s->audio_decoder = mobcam_aac_decoder_create(on_audio_decoded, s);
        }

        // Start media clients
        s->video_client = mobcam_media_client_create(target_host, 4748, on_video_raw_packet, s);
        mobcam_media_client_start(s->video_client);

        if (s->config.audio_enabled) {
            s->audio_client = mobcam_media_client_create(target_host, 4749, on_audio_raw_packet, s);
            mobcam_media_client_start(s->audio_client);
        }

        // Keepalive / monitor loop with seamless background USB/Wi-Fi auto-switching
        int ping_counter = 0;
        int check_counter = 0;
        char active_streaming_ip[128];
        strncpy(active_streaming_ip, target_host, sizeof(active_streaming_ip) - 1);
        bool active_is_usb = is_usb;

        while (s->running &&
               mobcam_control_is_connected(s->control_client) &&
               mobcam_media_client_is_running(s->video_client)) {
            SLEEP_MS(50);
            mobcam_control_poll_events(s->control_client);

            ping_counter++;
            if (ping_counter >= 100) { // Every 5 seconds (100 * 50ms)
                mobcam_control_poll_heartbeat(s->control_client);
                ping_counter = 0;
            }

            // Seamless Network Migration:
            // If currently on Wi-Fi, detect if USB Wired connection becomes active
            check_counter++;
            if (check_counter >= 20 && strcmp(s->device_selection, "manual") != 0) { // Every 1s (20 * 50ms)
                check_counter = 0;
                char pref_ip[64] = {0};
                int pref_port = 4747;
                bool pref_usb = false;
                if (mobcam_discovery_get_preferred_ip(s->device_selection, pref_ip, sizeof(pref_ip), &pref_port, &pref_usb)) {
                    if (!active_is_usb && pref_usb && strcmp(pref_ip, active_streaming_ip) != 0) {
                        blog(LOG_INFO, "[MobCam] USB Wired network detected (%s)! Seamlessly migrating stream from Wi-Fi (%s)...",
                             pref_ip, active_streaming_ip);
                        break; // Break monitor loop to hot-switch to low-latency USB
                    }
                }
            }
        }

        blog(LOG_INFO, "[MobCam] Connection transitioning. Cleaning up session...");

        // Teardown previous session
        if (s->video_client) {
            mobcam_media_client_destroy(s->video_client);
            s->video_client = NULL;
        }
        if (s->audio_client) {
            mobcam_media_client_destroy(s->audio_client);
            s->audio_client = NULL;
        }
        MUTEX_LOCK(&s->video_mutex);
        mobcam_h264_decoder_t *vdec = s->video_decoder;
        s->video_decoder = NULL;
        MUTEX_UNLOCK(&s->video_mutex);
        if (vdec) {
            mobcam_h264_decoder_destroy(vdec);
        }

        MUTEX_LOCK(&s->audio_mutex);
        mobcam_aac_decoder_t *adec = s->audio_decoder;
        s->audio_decoder = NULL;
        MUTEX_UNLOCK(&s->audio_mutex);
        if (adec) {
            mobcam_aac_decoder_destroy(adec);
        }
        if (s->control_client) {
            mobcam_control_destroy(s->control_client);
            s->control_client = NULL;
        }

        if (s->running) {
            // Fast reconnection delay (150ms) for smooth handoff
            SLEEP_MS(150);
        }
    }

    return 0;
}

static void read_settings(mobcam_source_data_t *s, obs_data_t *settings) {
    const char *sel = obs_data_get_string(settings, "device_selection");
    if (sel && strlen(sel) > 0) {
        strncpy(s->device_selection, sel, sizeof(s->device_selection) - 1);
    } else {
        strncpy(s->device_selection, "auto", sizeof(s->device_selection) - 1);
    }

    if (strcmp(s->device_selection, "manual") != 0) {
        char target_host[64] = {0};
        int target_port = 4747;
        bool is_usb = false;
        if (mobcam_discovery_get_preferred_ip(s->device_selection, target_host, sizeof(target_host), &target_port, &is_usb)) {
            strncpy(s->host, target_host, sizeof(s->host) - 1);
            s->port = target_port;
            s->usb_mode = is_usb;
        } else {
            strncpy(s->host, obs_data_get_string(settings, "host"), sizeof(s->host) - 1);
            s->port = (int)obs_data_get_int(settings, "port");
            s->usb_mode = obs_data_get_bool(settings, "usb_mode");
        }
    } else {
        strncpy(s->host, obs_data_get_string(settings, "host"), sizeof(s->host) - 1);
        s->port = (int)obs_data_get_int(settings, "port");
        s->usb_mode = obs_data_get_bool(settings, "usb_mode");
    }

    if (s->port <= 0) s->port = 4747;
    if (strlen(s->host) == 0) strncpy(s->host, "127.0.0.1", sizeof(s->host) - 1);

    if (strlen(s->config.resolution) == 0) {
        strncpy(s->config.resolution, "1920x1080", sizeof(s->config.resolution) - 1);
        s->config.width = 1920;
        s->config.height = 1080;
    }
    if (s->config.fps <= 0) s->config.fps = 30;
    if (s->config.bitrate_kbps <= 0) s->config.bitrate_kbps = 8000;
    s->config.audio_enabled = true;

    double z = obs_data_get_double(settings, "zoom");
    s->zoom = (z <= 0.0) ? 1.0f : (float)z;
    s->torch = obs_data_get_bool(settings, "torch");
    s->exposure = (int)obs_data_get_int(settings, "exposure");
    const char *wb_val = obs_data_get_string(settings, "wb");
    if (wb_val && strlen(wb_val) > 0) {
        strncpy(s->wb, wb_val, sizeof(s->wb) - 1);
    } else if (strlen(s->wb) == 0) {
        strncpy(s->wb, "auto", sizeof(s->wb) - 1);
    }
    const char *cam_val = obs_data_get_string(settings, "camera");
    if (cam_val && strlen(cam_val) > 0) {
        strncpy(s->config.camera, cam_val, sizeof(s->config.camera) - 1);
    } else if (strlen(s->config.camera) == 0) {
        strncpy(s->config.camera, "back", sizeof(s->config.camera) - 1);
    }

    if (s->source) {
        obs_source_set_async_unbuffered(s->source, s->ultra_low_latency);
    }
}

static void *mobcam_create(obs_data_t *settings, obs_source_t *source) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)calloc(1, sizeof(mobcam_source_data_t));
    if (!s) return NULL;

    s->source = source;
    s->ultra_low_latency = true;
    MUTEX_INIT(&s->clock_mutex);
    MUTEX_INIT(&s->video_mutex);
    MUTEX_INIT(&s->audio_mutex);
    read_settings(s, settings);

    s->running = true;
#ifdef _WIN32
    s->worker_thread = CreateThread(NULL, 0, mobcam_worker_loop, s, 0, NULL);
#else
    pthread_create(&s->worker_thread, NULL, mobcam_worker_loop, s);
#endif

    init_sources_registry();
    MUTEX_LOCK(&s_sources_mutex);
    if (s_sources_count < MAX_MOBCAM_SOURCES) {
        s_sources[s_sources_count++] = s;
    }
    MUTEX_UNLOCK(&s_sources_mutex);
    mobcam_web_server_set_callbacks(mobcam_source_query_status, mobcam_source_apply_ctrl);

    return s;
}

static void mobcam_destroy(void *data) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)data;
    if (!s) return;

    init_sources_registry();
    MUTEX_LOCK(&s_sources_mutex);
    for (size_t i = 0; i < s_sources_count; i++) {
        if (s_sources[i] == s) {
            for (size_t j = i; j + 1 < s_sources_count; j++) {
                s_sources[j] = s_sources[j + 1];
            }
            s_sources_count--;
            break;
        }
    }
    if (s_sources_count == 0) {
        mobcam_web_server_set_callbacks(NULL, NULL);
    }
    MUTEX_UNLOCK(&s_sources_mutex);

    s->running = false;

    // Disconnect sockets to unblock any pending network read
    if (s->control_client) {
        mobcam_control_disconnect(s->control_client);
    }
    if (s->video_client) {
        mobcam_media_client_stop(s->video_client);
    }
    if (s->audio_client) {
        mobcam_media_client_stop(s->audio_client);
    }

#ifdef _WIN32
    if (s->worker_thread) {
        WaitForSingleObject(s->worker_thread, 5000);
        CloseHandle(s->worker_thread);
        s->worker_thread = NULL;
    }
#else
    if (s->worker_thread) {
        pthread_join(s->worker_thread, NULL);
    }
#endif

    if (s->video_client) {
        mobcam_media_client_destroy(s->video_client);
        s->video_client = NULL;
    }
    if (s->audio_client) {
        mobcam_media_client_destroy(s->audio_client);
        s->audio_client = NULL;
    }

    MUTEX_LOCK(&s->video_mutex);
    mobcam_h264_decoder_t *vdec = s->video_decoder;
    s->video_decoder = NULL;
    MUTEX_UNLOCK(&s->video_mutex);
    if (vdec) {
        mobcam_h264_decoder_destroy(vdec);
    }

    MUTEX_LOCK(&s->audio_mutex);
    mobcam_aac_decoder_t *adec = s->audio_decoder;
    s->audio_decoder = NULL;
    MUTEX_UNLOCK(&s->audio_mutex);
    if (adec) {
        mobcam_aac_decoder_destroy(adec);
    }

    if (s->control_client) {
        mobcam_control_destroy(s->control_client);
        s->control_client = NULL;
    }

    mobcam_watermark_backup_free(&s->watermark_backup);

    MUTEX_DESTROY(&s->video_mutex);
    MUTEX_DESTROY(&s->audio_mutex);
    MUTEX_DESTROY(&s->clock_mutex);
    free(s);
}

static void mobcam_update(void *data, obs_data_t *settings) {
    mobcam_source_data_t *s = (mobcam_source_data_t *)data;
    if (!s || s->in_internal_update) return;

    char old_host[128] = {0};
    int old_port = s->port;
    strncpy(old_host, s->host, sizeof(old_host) - 1);

    read_settings(s, settings);

    // If target host/port changed, disconnect so worker reconnects immediately
    if (strcmp(old_host, s->host) != 0 || old_port != s->port) {
        blog(LOG_INFO, "[MobCam] Target host/port changed to %s:%d, reconnecting...", s->host, s->port);
        if (s->control_client) {
            mobcam_control_disconnect(s->control_client);
        }
        return;
    }

    // If currently connected, send live config and camera control update to phone
    if (s->control_client && mobcam_control_is_connected(s->control_client)) {
        mobcam_config_t ack;
        mobcam_control_send_config(s->control_client, &s->config, &ack, 2000);
        mobcam_control_send_camera_control(s->control_client, s->zoom, s->torch, s->exposure, s->config.camera, s->wb);
    }
}

struct obs_source_info mobcam_source_info = {
    .id = "mobcam_source",
    .type = OBS_SOURCE_TYPE_INPUT,
    .output_flags = OBS_SOURCE_ASYNC_VIDEO | OBS_SOURCE_AUDIO | OBS_SOURCE_DO_NOT_DUPLICATE,
    .get_name = mobcam_get_name,
    .create = mobcam_create,
    .destroy = mobcam_destroy,
    .update = mobcam_update,
    .get_defaults = mobcam_get_defaults,
    .get_properties = mobcam_get_properties,
    .icon_type = OBS_ICON_TYPE_CAMERA,
};
