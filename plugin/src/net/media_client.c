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
#include "media_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define CLOSE_SOCKET(s) closesocket(s)
#define THREAD_HANDLE HANDLE
#define THREAD_FUNC DWORD WINAPI
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define CLOSE_SOCKET(s) close(s)
#define THREAD_HANDLE pthread_t
#define THREAD_FUNC void *
#endif

struct mobcam_media_client {
    char host[128];
    int port;
    mobcam_frame_callback_t frame_cb;
    void *user_data;

    SOCKET sock;
    volatile bool running;
    THREAD_HANDLE thread;
};

static bool recv_exact(SOCKET s, uint8_t *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        int n = recv(s, (char *)buf + total, (int)(len - total), 0);
        if (n <= 0) return false;
        total += (size_t)n;
    }
    return true;
}

static THREAD_FUNC media_worker_thread(void *param) {
    mobcam_media_client_t *c = (mobcam_media_client_t *)param;
    uint8_t *frame_buffer = (uint8_t *)malloc(4 * 1024 * 1024 + 128); // 4MB buffer + 128B padding for SIMD
    if (!frame_buffer) return 0;

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", c->port);

    if (getaddrinfo(c->host, port_str, &hints, &res) != 0 || !res) {
        free(frame_buffer);
        c->running = false;
        return 0;
    }

    c->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (c->sock == INVALID_SOCKET) {
        freeaddrinfo(res);
        free(frame_buffer);
        c->running = false;
        return 0;
    }

    int flag = 1;
    setsockopt(c->sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
    int rcvbuf = 256 * 1024;
    setsockopt(c->sock, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(int));

    if (connect(c->sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        freeaddrinfo(res);
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
        free(frame_buffer);
        c->running = false;
        return 0;
    }
    freeaddrinfo(res);

#ifdef _WIN32
    DWORD tv_ms = 2000;
    setsockopt(c->sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_ms, sizeof(tv_ms));
#else
    struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
    setsockopt(c->sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
#endif

    while (c->running) {
        uint8_t header[12];
        if (!recv_exact(c->sock, header, 12)) {
            break;
        }

        uint32_t length = ((uint32_t)header[0] << 24) |
                          ((uint32_t)header[1] << 16) |
                          ((uint32_t)header[2] << 8)  |
                          ((uint32_t)header[3]);

        int64_t pts_us = ((int64_t)header[4] << 56) |
                         ((int64_t)header[5] << 48) |
                         ((int64_t)header[6] << 40) |
                         ((int64_t)header[7] << 32) |
                         ((int64_t)header[8] << 24) |
                         ((int64_t)header[9] << 16) |
                         ((int64_t)header[10] << 8) |
                         ((int64_t)header[11]);

        if (length == 0 || length > 4 * 1024 * 1024) {
            // Bad frame size or protocol corruption
            break;
        }

        if (!recv_exact(c->sock, frame_buffer, length)) {
            break;
        }
        memset(frame_buffer + length, 0, 64);

        if (c->frame_cb && c->running) {
            c->frame_cb(c->user_data, frame_buffer, length, pts_us);
        }
    }

    if (c->sock != INVALID_SOCKET) {
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
    }
    free(frame_buffer);
    c->running = false;
    return 0;
}

mobcam_media_client_t *mobcam_media_client_create(const char *host, int port,
                                                  mobcam_frame_callback_t frame_cb,
                                                  void *user_data) {
    mobcam_media_client_t *c = (mobcam_media_client_t *)calloc(1, sizeof(mobcam_media_client_t));
    if (!c) return NULL;

    strncpy(c->host, host ? host : "127.0.0.1", sizeof(c->host) - 1);
    c->port = port;
    c->frame_cb = frame_cb;
    c->user_data = user_data;
    c->sock = INVALID_SOCKET;
    c->running = false;
    return c;
}

bool mobcam_media_client_start(mobcam_media_client_t *c) {
    if (!c || c->running) return false;
    c->running = true;

#ifdef _WIN32
    c->thread = CreateThread(NULL, 0, media_worker_thread, c, 0, NULL);
    return (c->thread != NULL);
#else
    return (pthread_create(&c->thread, NULL, media_worker_thread, c) == 0);
#endif
}

void mobcam_media_client_stop(mobcam_media_client_t *c) {
    if (!c) return;
    c->running = false;

    if (c->sock != INVALID_SOCKET) {
        // Force unblock recv
        CLOSE_SOCKET(c->sock);
        c->sock = INVALID_SOCKET;
    }

#ifdef _WIN32
    if (c->thread != NULL) {
        WaitForSingleObject(c->thread, 2000);
        CloseHandle(c->thread);
        c->thread = NULL;
    }
#else
    if (c->thread) {
        pthread_join(c->thread, NULL);
        c->thread = 0;
    }
#endif
}

bool mobcam_media_client_is_running(mobcam_media_client_t *c) {
    return c && c->running;
}

void mobcam_media_client_destroy(mobcam_media_client_t *c) {
    if (!c) return;
    mobcam_media_client_stop(c);
    free(c);
}
