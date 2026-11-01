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
#include "control_client.h"
#include "../cjson/cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#define CLOSE_SOCKET(s) closesocket(s)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define CLOSE_SOCKET(s) close(s)
#endif

#define BUFFER_SIZE 4096

struct mobcam_control_client {
    char host[128];
    int port;
    char pin[16];
    SOCKET sock;
    bool connected;
    bool paired;
    char rx_buffer[BUFFER_SIZE];
    size_t rx_len;
    mobcam_state_callback_t state_cb;
    mobcam_stream_info_callback_t stream_info_cb;
    void *user_data;
};

static void init_network(void) {
#ifdef _WIN32
    static int initialized = 0;
    if (!initialized) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        initialized = 1;
    }
#endif
}

mobcam_control_client_t *mobcam_control_create(const char *host, int port, const char *pin,
                                               mobcam_state_callback_t state_cb,
                                               mobcam_stream_info_callback_t stream_info_cb,
                                               void *user_data) {
    init_network();
    mobcam_control_client_t *c = (mobcam_control_client_t *)calloc(1, sizeof(mobcam_control_client_t));
    if (!c) return NULL;

    strncpy(c->host, host ? host : "127.0.0.1", sizeof(c->host) - 1);
    c->port = port > 0 ? port : 4747;
    strncpy(c->pin, pin ? pin : "", sizeof(c->pin) - 1);
    c->sock = INVALID_SOCKET;
    c->state_cb = state_cb;
    c->stream_info_cb = stream_info_cb;
    c->user_data = user_data;
    return c;
}

static bool socket_send_line(SOCKET s, const char *json_str) {
    size_t len = strlen(json_str);
    char *buf = (char *)malloc(len + 2);
    if (!buf) return false;
    memcpy(buf, json_str, len);
    buf[len] = '\n';
    buf[len + 1] = '\0';

    int sent = send(s, buf, (int)(len + 1), 0);
    free(buf);
    return (sent == (int)(len + 1));
}

static char *socket_recv_line(mobcam_control_client_t *c, int timeout_ms) {
    if (c->sock == INVALID_SOCKET) return NULL;

    fd_set read_fds;
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    while (1) {
        // Check if newline already in rx_buffer
        for (size_t i = 0; i < c->rx_len; i++) {
            if (c->rx_buffer[i] == '\n') {
                char *line = (char *)malloc(i + 1);
                if (!line) return NULL;
                memcpy(line, c->rx_buffer, i);
                line[i] = '\0';

                // Shift remainder
                size_t remainder = c->rx_len - (i + 1);
                memmove(c->rx_buffer, c->rx_buffer + i + 1, remainder);
                c->rx_len = remainder;
                return line;
            }
        }

        FD_ZERO(&read_fds);
        FD_SET(c->sock, &read_fds);
        int res = select((int)(c->sock + 1), &read_fds, NULL, NULL, &tv);
        if (res <= 0) return NULL; // timeout or error

        int bytes = recv(c->sock, c->rx_buffer + c->rx_len, (int)(sizeof(c->rx_buffer) - c->rx_len - 1), 0);
        if (bytes <= 0) {
            c->connected = false;
            CLOSE_SOCKET(c->sock);
            c->sock = INVALID_SOCKET;
            return NULL;
        }
        c->rx_len += (size_t)bytes;
        c->rx_buffer[c->rx_len] = '\0';
    }
}

bool mobcam_control_connect_and_pair(mobcam_control_client_t *c, int timeout_ms) {
    if (!c) return false;
    if (c->sock != INVALID_SOCKET) {
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
    }
    c->connected = false;
    c->paired = false;
    c->rx_len = 0;

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", c->port);

    if (getaddrinfo(c->host, port_str, &hints, &res) != 0 || !res) {
        return false;
    }

    c->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (c->sock == INVALID_SOCKET) {
        freeaddrinfo(res);
        return false;
    }

    // Set TCP_NODELAY
    int flag = 1;
    setsockopt(c->sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    if (connect(c->sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        freeaddrinfo(res);
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
        return false;
    }
    freeaddrinfo(res);
    c->connected = true;

    // 1. Read 'hello' from phone
    char *hello_line = socket_recv_line(c, timeout_ms);
    if (!hello_line) {
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
        c->connected = false;
        return false;
    }
    cJSON *hello_json = cJSON_Parse(hello_line);
    free(hello_line);
    if (!hello_json) {
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
        c->connected = false;
        return false;
    }

    if (c->state_cb) {
        cJSON *pro_item = cJSON_GetObjectItem(hello_json, "is_pro");
        cJSON *rot_item = cJSON_GetObjectItem(hello_json, "rotation");
        bool is_pro = pro_item && cJSON_IsTrue(pro_item);
        int rot = rot_item ? rot_item->valueint : 0;
        c->state_cb(c->user_data, "back", false, -1, is_pro, rot, 1.0f, 0, "auto");
    }

    if (c->stream_info_cb) {
        cJSON *res_item = cJSON_GetObjectItem(hello_json, "resolution");
        cJSON *fps_item = cJSON_GetObjectItem(hello_json, "fps");
        cJSON *bitrate_item = cJSON_GetObjectItem(hello_json, "bitrate_kbps");
        cJSON *lat_item = cJSON_GetObjectItem(hello_json, "latency_mode");
        cJSON *audio_item = cJSON_GetObjectItem(hello_json, "audio");
        cJSON *codec_item = cJSON_GetObjectItem(hello_json, "codec");

        const char *res = (res_item && res_item->valuestring) ? res_item->valuestring : "1920x1080";
        int fps = fps_item ? fps_item->valueint : 30;
        int bitrate = bitrate_item ? bitrate_item->valueint : 8000;
        const char *lat_mode = (lat_item && lat_item->valuestring) ? lat_item->valuestring : "ultra_low";
        bool audio = audio_item ? cJSON_IsTrue(audio_item) : true;
        const char *codec = (codec_item && codec_item->valuestring) ? codec_item->valuestring : "h264";

        c->stream_info_cb(c->user_data, res, fps, bitrate, lat_mode, audio, codec);
    }
    cJSON_Delete(hello_json);

    // 2. Send 'pair_request'
    cJSON *pair_req = cJSON_CreateObject();
    cJSON_AddStringToObject(pair_req, "type", "pair_request");
    cJSON_AddStringToObject(pair_req, "pin", c->pin);
    char *pair_req_str = cJSON_PrintUnformatted(pair_req);
    socket_send_line(c->sock, pair_req_str);
    free(pair_req_str);
    cJSON_Delete(pair_req);

    // 3. Read 'pair_result'
    char *pair_res_line = socket_recv_line(c, timeout_ms);
    if (!pair_res_line) return false;

    cJSON *pair_res_json = cJSON_Parse(pair_res_line);
    free(pair_res_line);
    if (!pair_res_json) return false;

    cJSON *ok_item = cJSON_GetObjectItem(pair_res_json, "ok");
    bool ok = ok_item && cJSON_IsTrue(ok_item);
    cJSON_Delete(pair_res_json);

    c->paired = ok;
    return ok;
}

static void handle_control_message(mobcam_control_client_t *c, const char *line);

bool mobcam_control_send_config(mobcam_control_client_t *c, const mobcam_config_t *config,
                                mobcam_config_t *out_negotiated, int timeout_ms) {
    if (!c || c->sock == INVALID_SOCKET || !c->paired) return false;

    cJSON *cfg = cJSON_CreateObject();
    cJSON_AddStringToObject(cfg, "type", "set_config");
    cJSON_AddStringToObject(cfg, "resolution", config->resolution);
    cJSON_AddNumberToObject(cfg, "fps", config->fps);
    cJSON_AddNumberToObject(cfg, "bitrate_kbps", config->bitrate_kbps);
    cJSON_AddBoolToObject(cfg, "audio", config->audio_enabled);
    cJSON_AddStringToObject(cfg, "camera", config->camera);
    cJSON_AddStringToObject(cfg, "codec", (config->codec && strlen(config->codec) > 0) ? config->codec : "h264");

    char *cfg_str = cJSON_PrintUnformatted(cfg);
    socket_send_line(c->sock, cfg_str);
    free(cfg_str);
    cJSON_Delete(cfg);

    while (1) {
        char *ack_line = socket_recv_line(c, timeout_ms);
        if (!ack_line) return false;

        cJSON *ack = cJSON_Parse(ack_line);
        if (!ack) {
            free(ack_line);
            return false;
        }

        cJSON *type_item = cJSON_GetObjectItem(ack, "type");
        if (type_item && strcmp(type_item->valuestring, "config_ack") == 0) {
            if (out_negotiated) {
                cJSON *res_item = cJSON_GetObjectItem(ack, "resolution");
                cJSON *fps_item = cJSON_GetObjectItem(ack, "fps");
                cJSON *br_item = cJSON_GetObjectItem(ack, "bitrate_kbps");
                cJSON *aud_item = cJSON_GetObjectItem(ack, "audio");

                if (res_item && res_item->valuestring) {
                    strncpy(out_negotiated->resolution, res_item->valuestring, sizeof(out_negotiated->resolution) - 1);
                    sscanf(out_negotiated->resolution, "%dx%d", &out_negotiated->width, &out_negotiated->height);
                }
                if (fps_item) out_negotiated->fps = fps_item->valueint;
                if (br_item) out_negotiated->bitrate_kbps = br_item->valueint;
                if (aud_item) out_negotiated->audio_enabled = cJSON_IsTrue(aud_item);
            }
            cJSON_Delete(ack);
            free(ack_line);
            return true;
        }

        // Interleaved broadcast (e.g. state or stream_info): handle it and continue waiting for config_ack
        handle_control_message(c, ack_line);
        cJSON_Delete(ack);
        free(ack_line);
    }
}

static void handle_control_message(mobcam_control_client_t *c, const char *line) {
    if (!c || !line) return;
    cJSON *msg = cJSON_Parse(line);
    if (!msg) return;

    cJSON *type_item = cJSON_GetObjectItem(msg, "type");
    if (type_item && strcmp(type_item->valuestring, "state") == 0) {
        cJSON *cam = cJSON_GetObjectItem(msg, "camera");
        cJSON *torch = cJSON_GetObjectItem(msg, "torch");
        cJSON *batt = cJSON_GetObjectItem(msg, "battery_pct");
        cJSON *pro_item = cJSON_GetObjectItem(msg, "is_pro");
        cJSON *rot_item = cJSON_GetObjectItem(msg, "rotation");
        cJSON *zoom_item = cJSON_GetObjectItem(msg, "zoom");
        cJSON *exp_item = cJSON_GetObjectItem(msg, "exposure");
        cJSON *wb_item = cJSON_GetObjectItem(msg, "wb");

        bool is_pro = pro_item && cJSON_IsTrue(pro_item);
        int rot = rot_item ? rot_item->valueint : 0;
        float zoom = zoom_item ? (float)zoom_item->valuedouble : 1.0f;
        int exp = exp_item ? exp_item->valueint : 0;
        const char *wb = (wb_item && wb_item->valuestring) ? wb_item->valuestring : "auto";

        if (c->state_cb) {
            c->state_cb(c->user_data,
                        (cam && cam->valuestring) ? cam->valuestring : "back",
                        torch && cJSON_IsTrue(torch),
                        batt ? batt->valueint : -1,
                        is_pro,
                        rot,
                        zoom,
                        exp,
                        wb);
        }
    } else if (type_item && strcmp(type_item->valuestring, "stream_info") == 0) {
        cJSON *res_item = cJSON_GetObjectItem(msg, "resolution");
        cJSON *fps_item = cJSON_GetObjectItem(msg, "fps");
        cJSON *bitrate_item = cJSON_GetObjectItem(msg, "bitrate_kbps");
        cJSON *lat_item = cJSON_GetObjectItem(msg, "latency_mode");
        cJSON *audio_item = cJSON_GetObjectItem(msg, "audio");
        cJSON *codec_item = cJSON_GetObjectItem(msg, "codec");

        const char *res = (res_item && res_item->valuestring) ? res_item->valuestring : "1920x1080";
        int fps = fps_item ? fps_item->valueint : 30;
        int bitrate = bitrate_item ? bitrate_item->valueint : 8000;
        const char *lat_mode = (lat_item && lat_item->valuestring) ? lat_item->valuestring : "ultra_low";
        bool audio = audio_item ? cJSON_IsTrue(audio_item) : true;
        const char *codec = (codec_item && codec_item->valuestring) ? codec_item->valuestring : "h264";

        if (c->stream_info_cb) {
            c->stream_info_cb(c->user_data, res, fps, bitrate, lat_mode, audio, codec);
        }
    }
    cJSON_Delete(msg);
}

void mobcam_control_poll_events(mobcam_control_client_t *c) {
    if (!c || c->sock == INVALID_SOCKET || !c->connected) return;

    char *line;
    while ((line = socket_recv_line(c, 0)) != NULL) {
        handle_control_message(c, line);
        free(line);
    }
}

void mobcam_control_poll_heartbeat(mobcam_control_client_t *c) {
    if (!c || c->sock == INVALID_SOCKET) return;

    // Send ping
    cJSON *ping = cJSON_CreateObject();
    cJSON_AddStringToObject(ping, "type", "ping");
    char *ping_str = cJSON_PrintUnformatted(ping);
    if (!socket_send_line(c->sock, ping_str)) {
        c->connected = false;
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
    }
    free(ping_str);
    cJSON_Delete(ping);

    // Read any pending incoming messages (pong or state)
    char *line;
    while ((line = socket_recv_line(c, 100)) != NULL) {
        handle_control_message(c, line);
        free(line);
    }
}

bool mobcam_control_is_connected(mobcam_control_client_t *c) {
    return c && c->connected && c->paired;
}

void mobcam_control_disconnect(mobcam_control_client_t *c) {
    if (!c) return;
    c->connected = false;
    c->paired = false;
    if (c->sock != INVALID_SOCKET) {
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
    }
}

bool mobcam_control_send_camera_control(mobcam_control_client_t *c, float zoom, bool torch, int exposure, const char *camera, const char *wb) {
    if (!c || c->sock == INVALID_SOCKET) return false;
    cJSON *cmd = cJSON_CreateObject();
    cJSON_AddStringToObject(cmd, "command", "camera_control");
    if (zoom >= 0.9f) cJSON_AddNumberToObject(cmd, "zoom", (double)zoom);
    cJSON_AddBoolToObject(cmd, "torch", torch);
    cJSON_AddNumberToObject(cmd, "exposure", exposure);
    if (camera && strlen(camera) > 0) cJSON_AddStringToObject(cmd, "camera", camera);
    if (wb && strlen(wb) > 0) cJSON_AddStringToObject(cmd, "wb", wb);

    char *cmd_str = cJSON_PrintUnformatted(cmd);
    bool sent = socket_send_line(c->sock, cmd_str);
    free(cmd_str);
    cJSON_Delete(cmd);
    return sent;
}

void mobcam_control_destroy(mobcam_control_client_t *c) {
    if (!c) return;
    mobcam_control_disconnect(c);
    free(c);
}
