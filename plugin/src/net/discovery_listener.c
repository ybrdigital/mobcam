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
#include "discovery_listener.h"
#include "cjson/cJSON.h"

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
#define MUTEX_HANDLE CRITICAL_SECTION
#define MUTEX_INIT(m) InitializeCriticalSection(m)
#define MUTEX_LOCK(m) EnterCriticalSection(m)
#define MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#define MUTEX_DESTROY(m) DeleteCriticalSection(m)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define CLOSE_SOCKET(s) close(s)
#define THREAD_HANDLE pthread_t
#define THREAD_FUNC void *
#define MUTEX_HANDLE pthread_mutex_t
#define MUTEX_INIT(m) pthread_mutex_init(m, NULL)
#define MUTEX_LOCK(m) pthread_mutex_lock(m)
#define MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#define MUTEX_DESTROY(m) pthread_mutex_destroy(m)
#endif

#define MAX_DISCOVERED 16
#define UDP_BEACON_PORT 4750

static mobcam_discovered_device_t s_devices[MAX_DISCOVERED];
static size_t s_device_count = 0;
static MUTEX_HANDLE s_mutex;
static volatile bool s_running = false;
static THREAD_HANDLE s_thread = NULL;
static SOCKET s_udp_sock = INVALID_SOCKET;

static uint64_t get_time_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000);
#endif
}

static void add_or_update_beacon(const char *id, const char *name, const char *ip, int port, bool is_usb) {
    MUTEX_LOCK(&s_mutex);
    uint64_t now = get_time_ms();

    // Find existing device by id
    for (size_t i = 0; i < s_device_count; i++) {
        if (strcmp(s_devices[i].id, id) == 0) {
            strncpy(s_devices[i].name, name, sizeof(s_devices[i].name) - 1);
            s_devices[i].port = port;
            if (is_usb) {
                strncpy(s_devices[i].usb_ip, ip, sizeof(s_devices[i].usb_ip) - 1);
                s_devices[i].last_seen_usb_ms = now;
            } else {
                strncpy(s_devices[i].wifi_ip, ip, sizeof(s_devices[i].wifi_ip) - 1);
                s_devices[i].last_seen_wifi_ms = now;
            }
            MUTEX_UNLOCK(&s_mutex);
            return;
        }
    }

    if (s_device_count < MAX_DISCOVERED) {
        mobcam_discovered_device_t *dev = &s_devices[s_device_count++];
        memset(dev, 0, sizeof(*dev));
        strncpy(dev->id, id, sizeof(dev->id) - 1);
        strncpy(dev->name, name, sizeof(dev->name) - 1);
        dev->port = port;
        if (is_usb) {
            strncpy(dev->usb_ip, ip, sizeof(dev->usb_ip) - 1);
            dev->last_seen_usb_ms = now;
        } else {
            strncpy(dev->wifi_ip, ip, sizeof(dev->wifi_ip) - 1);
            dev->last_seen_wifi_ms = now;
        }
    }
    MUTEX_UNLOCK(&s_mutex);
}

bool mobcam_discovery_get_preferred_ip(const char *device_id, char *out_ip, size_t max_ip_len, int *out_port, bool *out_is_usb) {
    MUTEX_LOCK(&s_mutex);
    uint64_t now = get_time_ms();

    for (size_t i = 0; i < s_device_count; i++) {
        mobcam_discovered_device_t *dev = &s_devices[i];
        bool matches = false;
        if (device_id == NULL || strlen(device_id) == 0 || strcmp(device_id, "auto") == 0) {
            matches = true;
        } else if (strcmp(dev->id, device_id) == 0) {
            matches = true;
        }

        if (matches) {
            // Prioritize USB network if seen within last 4000ms
            if (dev->usb_ip[0] != '\0' && (now - dev->last_seen_usb_ms < 4000)) {
                strncpy(out_ip, dev->usb_ip, max_ip_len - 1);
                out_ip[max_ip_len - 1] = '\0';
                if (out_port) *out_port = dev->port;
                if (out_is_usb) *out_is_usb = true;
                MUTEX_UNLOCK(&s_mutex);
                return true;
            }
            // Fallback to Wi-Fi network if seen within last 6000ms
            if (dev->wifi_ip[0] != '\0' && (now - dev->last_seen_wifi_ms < 6000)) {
                strncpy(out_ip, dev->wifi_ip, max_ip_len - 1);
                out_ip[max_ip_len - 1] = '\0';
                if (out_port) *out_port = dev->port;
                if (out_is_usb) *out_is_usb = false;
                MUTEX_UNLOCK(&s_mutex);
                return true;
            }
        }
    }

    MUTEX_UNLOCK(&s_mutex);
    return false;
}

static THREAD_FUNC discovery_worker(void *param) {
    (void)param;
    char buffer[1024];

    while (s_running) {
        struct sockaddr_in from_addr;
        int from_len = sizeof(from_addr);

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(s_udp_sock, &read_fds);
        struct timeval tv = {1, 0}; // 1 second select

        int res = select((int)(s_udp_sock + 1), &read_fds, NULL, NULL, &tv);
        if (res <= 0) continue;

        int bytes = recvfrom(s_udp_sock, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&from_addr, &from_len);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            cJSON *root = cJSON_Parse(buffer);
            if (root) {
                cJSON *app = cJSON_GetObjectItem(root, "app");
                if (app && app->valuestring && strcmp(app->valuestring, "mobcam") == 0) {
                    cJSON *id_item = cJSON_GetObjectItem(root, "id");
                    cJSON *name = cJSON_GetObjectItem(root, "name");
                    cJSON *port = cJSON_GetObjectItem(root, "port");
                    cJSON *transport = cJSON_GetObjectItem(root, "transport");
                    cJSON *ip_item = cJSON_GetObjectItem(root, "ip");

                    char from_ip[64] = {0};
                    if (ip_item && ip_item->valuestring && strlen(ip_item->valuestring) > 0) {
                        strncpy(from_ip, ip_item->valuestring, sizeof(from_ip) - 1);
                    } else {
                        inet_ntop(AF_INET, &from_addr.sin_addr, from_ip, sizeof(from_ip));
                    }

                    const char *dev_id = (id_item && id_item->valuestring) ? id_item->valuestring : from_ip;
                    const char *dev_name = (name && name->valuestring) ? name->valuestring : "MobCam Device";
                    int dev_port = (port && port->valueint > 0) ? port->valueint : 4747;
                    bool is_usb = (transport && transport->valuestring && strcmp(transport->valuestring, "usb") == 0);

                    add_or_update_beacon(dev_id, dev_name, from_ip, dev_port, is_usb);
                }
                cJSON_Delete(root);
            }
        }
    }

    return 0;
}

void mobcam_discovery_init(void) {
    if (s_running) return;

    MUTEX_INIT(&s_mutex);
    s_device_count = 0;

    s_udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s_udp_sock == INVALID_SOCKET) return;

    int opt = 1;
    setsockopt(s_udp_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(UDP_BEACON_PORT);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s_udp_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == SOCKET_ERROR) {
        CLOSE_SOCKET(s_udp_sock);
        s_udp_sock = INVALID_SOCKET;
        return;
    }

    s_running = true;
#ifdef _WIN32
    s_thread = CreateThread(NULL, 0, discovery_worker, NULL, 0, NULL);
#else
    pthread_create(&s_thread, NULL, discovery_worker, NULL);
#endif
}

void mobcam_discovery_cleanup(void) {
    if (!s_running) return;
    s_running = false;

    if (s_udp_sock != INVALID_SOCKET) {
        CLOSE_SOCKET(s_udp_sock);
        s_udp_sock = INVALID_SOCKET;
    }

    if (s_thread) {
#ifdef _WIN32
        WaitForSingleObject(s_thread, 2000);
        CloseHandle(s_thread);
#else
        pthread_join(s_thread, NULL);
#endif
        s_thread = NULL;
    }

    MUTEX_DESTROY(&s_mutex);
    s_device_count = 0;
}

size_t mobcam_discovery_get_devices(mobcam_discovered_device_t *out_devices, size_t max_count) {
    MUTEX_LOCK(&s_mutex);
    uint64_t now = get_time_ms();
    size_t count = 0;

    for (size_t i = 0; i < s_device_count && count < max_count; i++) {
        mobcam_discovered_device_t *dev = &s_devices[i];
        bool usb_fresh = (dev->usb_ip[0] != '\0' && (now - dev->last_seen_usb_ms < 8000));
        bool wifi_fresh = (dev->wifi_ip[0] != '\0' && (now - dev->last_seen_wifi_ms < 8000));

        if (usb_fresh || wifi_fresh) {
            memcpy(&out_devices[count++], dev, sizeof(mobcam_discovered_device_t));
        }
    }

    MUTEX_UNLOCK(&s_mutex);
    return count;
}
