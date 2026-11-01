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
#include "web_control_server.h"
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
#endif

static volatile bool s_server_running = false;
static SOCKET s_listen_sock = INVALID_SOCKET;
static THREAD_HANDLE s_server_thread = NULL;

static mobcam_web_query_multi_status_cb s_query_cb = NULL;
static mobcam_web_apply_ctrl_cb s_apply_cb = NULL;

static const char *HTML_PAGE = 
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"<meta charset=\"UTF-8\">\n"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"<title>MobCam Quick Controls</title>\n"
"<link rel=\"icon\" type=\"image/png\" href=\"data:image/png;base64,"
"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"
"\">\n"
"<style>\n"
"  * { box-sizing: border-box; margin: 0; padding: 0; user-select: none; }\n"
"  body {\n"
"    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;\n"
"    background: #11111b;\n"
"    color: #cdd6f4;\n"
"    padding: 10px;\n"
"    font-size: 13px;\n"
"  }\n"
"  .panel {\n"
"    background: #181825;\n"
"    border: 1px solid #313244;\n"
"    border-radius: 12px;\n"
"    padding: 12px;\n"
"    max-width: 420px;\n"
"    margin: 0 auto;\n"
"    display: flex;\n"
"    flex-direction: column;\n"
"    gap: 12px;\n"
"  }\n"
"  .header {\n"
"    display: flex;\n"
"    justify-content: space-between;\n"
"    align-items: center;\n"
"    border-bottom: 1px solid #313244;\n"
"    padding-bottom: 8px;\n"
"  }\n"
"  .brand {\n"
"    font-weight: 700;\n"
"    font-size: 14px;\n"
"    display: flex;\n"
"    align-items: center;\n"
"    gap: 8px;\n"
"    color: #cdd6f4;\n"
"  }\n"
"  .brand-logo {\n"
"    width: 22px;\n"
"    height: 22px;\n"
"    border-radius: 5px;\n"
"    object-fit: contain;\n"
"    vertical-align: middle;\n"
"  }\n"
"  .badge {\n"
"    font-size: 11px;\n"
"    font-weight: 600;\n"
"    padding: 2px 8px;\n"
"    border-radius: 10px;\n"
"    background: #313244;\n"
"    color: #a6adc8;\n"
"    display: flex;\n"
"    align-items: center;\n"
"    gap: 5px;\n"
"  }\n"
"  .badge.live {\n"
"    background: rgba(166, 227, 161, 0.15);\n"
"    color: #a6e3a1;\n"
"    border: 1px solid rgba(166, 227, 161, 0.3);\n"
"  }\n"
"  .dot {\n"
"    width: 6px;\n"
"    height: 6px;\n"
"    border-radius: 50%;\n"
"    background: currentColor;\n"
"  }\n"
"  .section {\n"
"    display: flex;\n"
"    flex-direction: column;\n"
"    gap: 6px;\n"
"  }\n"
"  .sec-title {\n"
"    display: flex;\n"
"    justify-content: space-between;\n"
"    font-size: 12px;\n"
"    font-weight: 600;\n"
"    color: #a6adc8;\n"
"    text-transform: uppercase;\n"
"    letter-spacing: 0.5px;\n"
"  }\n"
"  .val-tag {\n"
"    color: #89b4fa;\n"
"    font-family: monospace;\n"
"    font-size: 13px;\n"
"    font-weight: 700;\n"
"  }\n"
"  .slider-row {\n"
"    display: flex;\n"
"    align-items: center;\n"
"    gap: 8px;\n"
"  }\n"
"  input[type=range] {\n"
"    flex: 1;\n"
"    accent-color: #89b4fa;\n"
"    cursor: pointer;\n"
"    height: 6px;\n"
"  }\n"
"  .stepper-btn {\n"
"    background: #313244;\n"
"    border: 1px solid #45475a;\n"
"    color: #cdd6f4;\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    width: 28px;\n"
"    height: 28px;\n"
"    border-radius: 6px;\n"
"    cursor: pointer;\n"
"    display: flex;\n"
"    align-items: center;\n"
"    justify-content: center;\n"
"    transition: background 0.15s;\n"
"  }\n"
"  .stepper-btn:hover {\n"
"    background: #45475a;\n"
"  }\n"
"  .pill-group {\n"
"    display: grid;\n"
"    grid-template-columns: repeat(5, 1fr);\n"
"    gap: 4px;\n"
"  }\n"
"  .pill-btn {\n"
"    background: #1e1e2e;\n"
"    border: 1px solid #313244;\n"
"    color: #a6adc8;\n"
"    padding: 6px 2px;\n"
"    border-radius: 6px;\n"
"    font-size: 11px;\n"
"    font-weight: 600;\n"
"    cursor: pointer;\n"
"    text-align: center;\n"
"    transition: all 0.15s;\n"
"  }\n"
"  .pill-btn:hover, .pill-btn.active {\n"
"    background: #89b4fa;\n"
"    color: #11111b;\n"
"    border-color: #89b4fa;\n"
"  }\n"
"  .action-grid {\n"
"    display: grid;\n"
"    grid-template-columns: 1fr;\n"
"    gap: 8px;\n"
"  }\n"
"  .action-btn {\n"
"    background: #1e1e2e;\n"
"    border: 1px solid #313244;\n"
"    color: #cdd6f4;\n"
"    padding: 10px;\n"
"    border-radius: 8px;\n"
"    font-size: 12px;\n"
"    font-weight: 600;\n"
"    cursor: pointer;\n"
"    display: flex;\n"
"    align-items: center;\n"
"    justify-content: center;\n"
"    gap: 6px;\n"
"    transition: all 0.2s;\n"
"  }\n"
"  .action-btn:hover {\n"
"    background: #313244;\n"
"  }\n"
"  .action-btn.torch-active {\n"
"    background: rgba(249, 226, 175, 0.2);\n"
"    color: #f9e2af;\n"
"    border: 1px solid #f9e2af;\n"
"    box-shadow: 0 0 10px rgba(249, 226, 175, 0.25);\n"
"  }\n"
"  .wb-grid {\n"
"    display: grid;\n"
"    grid-template-columns: repeat(5, 1fr);\n"
"    gap: 4px;\n"
"  }\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<div class=\"panel\">\n"
"  <div class=\"header\">\n"
"    <div class=\"brand\"><img class=\"brand-logo\" src=\"data:image/png;base64,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\" alt=\"MobCam\"> MobCam Quick Controls</div>\n"
"    <div id=\"statusBadge\" class=\"badge\"><span class=\"dot\"></span> <span id=\"statusText\">Scanning...</span></div>\n"
"  </div>\n"
"\n"
"  <!-- Target Camera Selector (Multi-Device) -->\n"
"  <div class=\"section\" id=\"secCameraSelect\">\n"
"    <div class=\"sec-title\"><span>Target Camera</span></div>\n"
"    <select id=\"cameraSelect\" onchange=\"onCameraSelected(this.value)\" style=\"width:100%; background:#1e1e2e; border:1px solid #313244; color:#cdd6f4; padding:7px 10px; border-radius:8px; font-size:12px; font-weight:600; outline:none; cursor:pointer;\">\n"
"      <option value=\"0\">Scanning for MobCam...</option>\n"
"    </select>\n"
"  </div>\n"
"\n"
"  <!-- Zoom Control -->\n"
"  <div class=\"section\">\n"
"    <div class=\"sec-title\">\n"
"      <span>Camera Zoom</span>\n"
"      <span id=\"zoomVal\" class=\"val-tag\">1.0x</span>\n"
"    </div>\n"
"    <div class=\"slider-row\">\n"
"      <button class=\"stepper-btn\" onclick=\"adjustZoom(-0.2)\">−</button>\n"
"      <input type=\"range\" id=\"zoomSlider\" min=\"1.0\" max=\"8.0\" step=\"0.1\" value=\"1.0\" oninput=\"onZoomSlider(this.value)\">\n"
"      <button class=\"stepper-btn\" onclick=\"adjustZoom(0.2)\">+</button>\n"
"    </div>\n"
"    <div class=\"pill-group\">\n"
"      <button class=\"pill-btn\" onclick=\"setZoom(1.0)\">1.0x</button>\n"
"      <button class=\"pill-btn\" onclick=\"setZoom(1.5)\">1.5x</button>\n"
"      <button class=\"pill-btn\" onclick=\"setZoom(2.0)\">2.0x</button>\n"
"      <button class=\"pill-btn\" onclick=\"setZoom(3.0)\">3.0x</button>\n"
"      <button class=\"pill-btn\" onclick=\"setZoom(5.0)\">5.0x</button>\n"
"    </div>\n"
"  </div>\n"
"\n"
"  <!-- Quick Actions (Torch) -->\n"
"  <div class=\"action-grid\">\n"
"    <button id=\"btnTorch\" class=\"action-btn\" onclick=\"toggleTorch()\">\n"
"      <span>💡</span> <span id=\"torchLabel\">Torch OFF</span>\n"
"    </button>\n"
"  </div>\n"
"\n"
"  <!-- Exposure Compensation -->\n"
"  <div class=\"section\">\n"
"    <div class=\"sec-title\">\n"
"      <span>Exposure (EV)</span>\n"
"      <span id=\"evVal\" class=\"val-tag\">0 EV</span>\n"
"    </div>\n"
"    <div class=\"slider-row\">\n"
"      <button class=\"stepper-btn\" onclick=\"adjustExposure(-1)\">−</button>\n"
"      <input type=\"range\" id=\"evSlider\" min=\"-4\" max=\"4\" step=\"1\" value=\"0\" oninput=\"onExposureSlider(this.value)\">\n"
"      <button class=\"stepper-btn\" onclick=\"adjustExposure(1)\">+</button>\n"
"    </div>\n"
"  </div>\n"
"\n"
"  <!-- White Balance -->\n"
"  <div class=\"section\">\n"
"    <div class=\"sec-title\"><span>White Balance</span></div>\n"
"    <div class=\"wb-grid\">\n"
"      <button class=\"pill-btn\" id=\"wb-auto\" onclick=\"setWb('auto')\">Auto</button>\n"
"      <button class=\"pill-btn\" id=\"wb-daylight\" onclick=\"setWb('daylight')\">Day</button>\n"
"      <button class=\"pill-btn\" id=\"wb-cloudy\" onclick=\"setWb('cloudy')\">Cloud</button>\n"
"      <button class=\"pill-btn\" id=\"wb-fluorescent\" onclick=\"setWb('fluorescent')\">Fluo</button>\n"
"      <button class=\"pill-btn\" id=\"wb-incandescent\" onclick=\"setWb('incandescent')\">Incand</button>\n"
"    </div>\n"
"  </div>\n"
"</div>\n"
"\n"
"<script>\n"
"  let currentSources = [];\n"
"  let selectedSourceId = 0;\n"
"  let curState = { zoom: 1.0, torch: false, exposure: 0, camera: 'back', wb: 'auto', connected: false };\n"
"  let sendTimer = null;\n"
"\n"
"  function onCameraSelected(val) {\n"
"    selectedSourceId = parseInt(val);\n"
"    const src = currentSources.find(s => s.id === selectedSourceId);\n"
"    if (src) {\n"
"      curState.zoom = src.zoom || 1.0;\n"
"      curState.torch = src.torch || false;\n"
"      curState.exposure = src.exposure || 0;\n"
"      curState.camera = src.camera || 'back';\n"
"      curState.wb = src.wb || 'auto';\n"
"      curState.connected = src.connected || false;\n"
"      updateUI(curState);\n"
"    }\n"
"  }\n"
"\n"
"  function updateUI(s) {\n"
"    document.getElementById('zoomVal').innerText = s.zoom.toFixed(1) + 'x';\n"
"    document.getElementById('zoomSlider').value = s.zoom;\n"
"\n"
"    const torchBtn = document.getElementById('btnTorch');\n"
"    if (s.torch) {\n"
"      torchBtn.classList.add('torch-active');\n"
"      document.getElementById('torchLabel').innerText = 'Torch ON';\n"
"    } else {\n"
"      torchBtn.classList.remove('torch-active');\n"
"      document.getElementById('torchLabel').innerText = 'Torch OFF';\n"
"    }\n"
"\n"
"    document.getElementById('evVal').innerText = (s.exposure > 0 ? '+' : '') + s.exposure + ' EV';\n"
"    document.getElementById('evSlider').value = s.exposure;\n"
"\n"
"    const badge = document.getElementById('statusBadge');\n"
"    const statusText = document.getElementById('statusText');\n"
"    if (s.connected) {\n"
"      badge.className = 'badge live';\n"
"      statusText.innerText = 'LIVE';\n"
"    } else {\n"
"      badge.className = 'badge';\n"
"      statusText.innerText = 'STANDBY';\n"
"    }\n"
"\n"
"    ['auto','daylight','cloudy','fluorescent','incandescent'].forEach(mode => {\n"
"      const el = document.getElementById('wb-' + mode);\n"
"      if (el) {\n"
"        if (s.wb === mode) el.classList.add('active');\n"
"        else el.classList.remove('active');\n"
"      }\n"
"    });\n"
"  }\n"
"\n"
"  function postControl(payload) {\n"
"    payload.source_id = selectedSourceId;\n"
"    fetch('/api/control', {\n"
"      method: 'POST',\n"
"      headers: { 'Content-Type': 'application/json' },\n"
"      body: JSON.stringify(payload)\n"
"    }).catch(e => console.error(e));\n"
"  }\n"
"\n"
"  function setZoom(z) {\n"
"    curState.zoom = Math.max(1.0, Math.min(8.0, z));\n"
"    updateUI(curState);\n"
"    postControl({ zoom: curState.zoom });\n"
"  }\n"
"\n"
"  function adjustZoom(delta) {\n"
"    setZoom(curState.zoom + delta);\n"
"  }\n"
"\n"
"  function onZoomSlider(val) {\n"
"    curState.zoom = parseFloat(val);\n"
"    document.getElementById('zoomVal').innerText = curState.zoom.toFixed(1) + 'x';\n"
"    clearTimeout(sendTimer);\n"
"    sendTimer = setTimeout(() => postControl({ zoom: curState.zoom }), 40);\n"
"  }\n"
"\n"
"  function toggleTorch() {\n"
"    curState.torch = !curState.torch;\n"
"    updateUI(curState);\n"
"    postControl({ torch: curState.torch });\n"
"  }\n"
"\n"
"  function setExposure(ev) {\n"
"    curState.exposure = Math.max(-4, Math.min(4, parseInt(ev)));\n"
"    updateUI(curState);\n"
"    postControl({ exposure: curState.exposure });\n"
"  }\n"
"\n"
"  function adjustExposure(delta) {\n"
"    setExposure(curState.exposure + delta);\n"
"  }\n"
"\n"
"  function onExposureSlider(val) {\n"
"    curState.exposure = parseInt(val);\n"
"    document.getElementById('evVal').innerText = (curState.exposure > 0 ? '+' : '') + curState.exposure + ' EV';\n"
"    clearTimeout(sendTimer);\n"
"    sendTimer = setTimeout(() => postControl({ exposure: curState.exposure }), 40);\n"
"  }\n"
"\n"
"  function setWb(mode) {\n"
"    curState.wb = mode;\n"
"    updateUI(curState);\n"
"    postControl({ wb: curState.wb });\n"
"  }\n"
"\n"
"  function updateCameraDropdown(sources) {\n"
"    const sel = document.getElementById('cameraSelect');\n"
"    if (!sources || sources.length === 0) {\n"
"      sel.innerHTML = '<option value=\"0\">No MobCam Active in OBS</option>';\n"
"      return;\n"
"    }\n"
"    const sig = sources.map(s => s.id + ':' + s.name + ':' + s.connected).join('|');\n"
"    if (sel.getAttribute('data-sig') !== sig) {\n"
"      sel.setAttribute('data-sig', sig);\n"
"      let html = '';\n"
"      let found = false;\n"
"      sources.forEach(s => {\n"
"        const isSel = (s.id === selectedSourceId);\n"
"        if (isSel) found = true;\n"
"        html += `<option value=\"${s.id}\" ${isSel ? 'selected' : ''}>${s.name} ${s.connected ? '[LIVE]' : '[OFFLINE]'}</option>`;\n"
"      });\n"
"      if (!found && sources.length > 0) {\n"
"        selectedSourceId = sources[0].id;\n"
"      }\n"
"      sel.innerHTML = html;\n"
"    }\n"
"  }\n"
"\n"
"  function fetchStatus() {\n"
"    fetch('/api/status')\n"
"      .then(r => r.json())\n"
"      .then(data => {\n"
"        if (data && data.sources) {\n"
"          currentSources = data.sources;\n"
"          updateCameraDropdown(data.sources);\n"
"          let src = currentSources.find(s => s.id === selectedSourceId);\n"
"          if (!src && currentSources.length > 0) {\n"
"            src = currentSources[0];\n"
"            selectedSourceId = src.id;\n"
"          }\n"
"          if (src) {\n"
"            curState.connected = src.connected;\n"
"            if (document.activeElement.id !== 'zoomSlider') curState.zoom = src.zoom || 1.0;\n"
"            curState.torch = src.torch || false;\n"
"            if (document.activeElement.id !== 'evSlider') curState.exposure = src.exposure || 0;\n"
"            curState.camera = src.camera || 'back';\n"
"            curState.wb = src.wb || 'auto';\n"
"            updateUI(curState);\n"
"          } else {\n"
"            curState.connected = false;\n"
"            updateUI(curState);\n"
"          }\n"
"        }\n"
"      })\n"
"      .catch(err => {\n"
"        curState.connected = false;\n"
"        updateUI(curState);\n"
"      });\n"
"  }\n"
"\n"
"  setInterval(fetchStatus, 300);\n"
"  fetchStatus();\n"
"</script>\n"
"</body>\n"
"</html>\n";

static bool send_all(SOCKET sock, const char *buf, size_t len) {
    size_t total_sent = 0;
    while (total_sent < len) {
        int sent = send(sock, buf + total_sent, (int)(len - total_sent), 0);
        if (sent <= 0) return false;
        total_sent += (size_t)sent;
    }
    return true;
}

static void send_http_response(SOCKET sock, int status_code, const char *content_type, const char *body) {
    char header[512];
    size_t body_len = body ? strlen(body) : 0;
    const char *status_str = (status_code == 200) ? "OK" : (status_code == 404) ? "Not Found" : "Error";
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
             "Access-Control-Allow-Headers: Content-Type\r\n"
             "Connection: close\r\n\r\n",
             status_code, status_str, content_type, body_len);

    send_all(sock, header, strlen(header));
    if (body_len > 0) {
        send_all(sock, body, body_len);
    }
}

static void handle_client(SOCKET client_sock) {
    char req_buf[4096] = {0};
    int received = recv(client_sock, req_buf, sizeof(req_buf) - 1, 0);
    if (received <= 0) {
        CLOSE_SOCKET(client_sock);
        return;
    }
    req_buf[received] = '\0';

    char method[16] = {0};
    char path[256] = {0};
    sscanf(req_buf, "%15s %255s", method, path);

    if (strcmp(method, "OPTIONS") == 0) {
        send_http_response(client_sock, 200, "text/plain", "");
    } else if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
            send_http_response(client_sock, 200, "text/html; charset=utf-8", HTML_PAGE);
        } else if (strcmp(path, "/api/status") == 0) {
            mobcam_multi_status_t status;
            memset(&status, 0, sizeof(status));

            if (s_query_cb) {
                s_query_cb(&status);
            }

            cJSON *root = cJSON_CreateObject();
            cJSON_AddNumberToObject(root, "count", status.count);
            cJSON *sources_arr = cJSON_CreateArray();

            for (int i = 0; i < status.count && i < 16; i++) {
                cJSON *item = cJSON_CreateObject();
                cJSON_AddNumberToObject(item, "id", status.sources[i].id);
                cJSON_AddStringToObject(item, "name", status.sources[i].name);
                cJSON_AddStringToObject(item, "device", status.sources[i].host);
                cJSON_AddBoolToObject(item, "connected", status.sources[i].connected);
                cJSON_AddNumberToObject(item, "zoom", (double)status.sources[i].zoom);
                cJSON_AddNumberToObject(item, "max_zoom", (double)status.sources[i].max_zoom);
                cJSON_AddBoolToObject(item, "torch", status.sources[i].torch);
                cJSON_AddBoolToObject(item, "has_torch", status.sources[i].has_torch);
                cJSON_AddNumberToObject(item, "exposure", status.sources[i].exposure);
                cJSON_AddStringToObject(item, "camera", status.sources[i].camera);
                cJSON_AddStringToObject(item, "wb", status.sources[i].wb);
                cJSON_AddItemToArray(sources_arr, item);
            }
            cJSON_AddItemToObject(root, "sources", sources_arr);

            if (status.count > 0) {
                cJSON_AddBoolToObject(root, "active", true);
                cJSON_AddBoolToObject(root, "connected", status.sources[0].connected);
                cJSON_AddStringToObject(root, "device", status.sources[0].name);
                cJSON_AddNumberToObject(root, "zoom", (double)status.sources[0].zoom);
                cJSON_AddNumberToObject(root, "max_zoom", (double)status.sources[0].max_zoom);
                cJSON_AddBoolToObject(root, "torch", status.sources[0].torch);
                cJSON_AddBoolToObject(root, "has_torch", status.sources[0].has_torch);
                cJSON_AddNumberToObject(root, "exposure", status.sources[0].exposure);
                cJSON_AddStringToObject(root, "camera", status.sources[0].camera);
                cJSON_AddStringToObject(root, "wb", status.sources[0].wb);
            } else {
                cJSON_AddBoolToObject(root, "active", false);
                cJSON_AddBoolToObject(root, "connected", false);
                cJSON_AddStringToObject(root, "device", "None");
                cJSON_AddNumberToObject(root, "zoom", 1.0);
                cJSON_AddNumberToObject(root, "max_zoom", 8.0);
                cJSON_AddBoolToObject(root, "torch", false);
                cJSON_AddBoolToObject(root, "has_torch", true);
                cJSON_AddNumberToObject(root, "exposure", 0);
                cJSON_AddStringToObject(root, "camera", "back");
                cJSON_AddStringToObject(root, "wb", "auto");
            }

            char *json_str = cJSON_PrintUnformatted(root);
            send_http_response(client_sock, 200, "application/json", json_str ? json_str : "{}");
            if (json_str) cJSON_free(json_str);
            cJSON_Delete(root);
        } else {
            send_http_response(client_sock, 404, "text/plain", "Not Found");
        }
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/control") == 0) {
        char *body_start = strstr(req_buf, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            cJSON *json = cJSON_Parse(body_start);
            if (json) {
                int source_id = 0;
                float zoom = -1.0f;
                int torch = -1;
                int exposure = 999;
                const char *cam = NULL;
                const char *wb = NULL;

                cJSON *item_id = cJSON_GetObjectItem(json, "source_id");
                if (item_id && cJSON_IsNumber(item_id)) source_id = item_id->valueint;

                cJSON *item_zoom = cJSON_GetObjectItem(json, "zoom");
                if (item_zoom && cJSON_IsNumber(item_zoom)) zoom = (float)item_zoom->valuedouble;

                cJSON *item_torch = cJSON_GetObjectItem(json, "torch");
                if (item_torch && cJSON_IsBool(item_torch)) torch = cJSON_IsTrue(item_torch) ? 1 : 0;

                cJSON *item_exp = cJSON_GetObjectItem(json, "exposure");
                if (item_exp && cJSON_IsNumber(item_exp)) exposure = item_exp->valueint;

                cJSON *item_cam = cJSON_GetObjectItem(json, "camera");
                if (item_cam && cJSON_IsString(item_cam)) cam = item_cam->valuestring;

                cJSON *item_wb = cJSON_GetObjectItem(json, "wb");
                if (item_wb && cJSON_IsString(item_wb)) wb = item_wb->valuestring;

                if (s_apply_cb) {
                    s_apply_cb(source_id, zoom, torch, exposure, cam, wb);
                }
                cJSON_Delete(json);
            }
        }
        send_http_response(client_sock, 200, "application/json", "{\"ok\":true}");
    } else {
        send_http_response(client_sock, 404, "text/plain", "Not Found");
    }

    CLOSE_SOCKET(client_sock);
}

static THREAD_FUNC client_thread_entry(void *param) {
    SOCKET sock = (SOCKET)(uintptr_t)param;
    handle_client(sock);
    return 0;
}

static THREAD_FUNC server_worker(void *param) {
    (void)param;

    while (s_server_running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(s_listen_sock, &read_fds);
        struct timeval tv = {1, 0}; // 1s timeout for graceful loop

        int res = select((int)(s_listen_sock + 1), &read_fds, NULL, NULL, &tv);
        if (res <= 0 || !s_server_running) continue;

        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        SOCKET client_sock = accept(s_listen_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock == INVALID_SOCKET) continue;

#ifdef _WIN32
        DWORD timeout_ms = 3000;
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms));
        setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms));

        HANDLE h = CreateThread(NULL, 0, client_thread_entry, (void *)(uintptr_t)client_sock, 0, NULL);
        if (h) {
            CloseHandle(h);
        } else {
            handle_client(client_sock);
        }
#else
        struct timeval tv_timeout = {3, 0};
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv_timeout, sizeof(tv_timeout));
        setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (const void *)&tv_timeout, sizeof(tv_timeout));

        pthread_t pt;
        if (pthread_create(&pt, NULL, (void *(*)(void *))client_thread_entry, (void *)(uintptr_t)client_sock) == 0) {
            pthread_detach(pt);
        } else {
            handle_client(client_sock);
        }
#endif
    }
    return 0;
}

bool mobcam_web_server_start(int port) {
    if (s_server_running) return true;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    s_listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s_listen_sock == INVALID_SOCKET) return false;

    int opt = 1;
    setsockopt(s_listen_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to 0.0.0.0 so phone or local PC can view

    if (bind(s_listen_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == SOCKET_ERROR) {
        CLOSE_SOCKET(s_listen_sock);
        s_listen_sock = INVALID_SOCKET;
        return false;
    }

    if (listen(s_listen_sock, 8) == SOCKET_ERROR) {
        CLOSE_SOCKET(s_listen_sock);
        s_listen_sock = INVALID_SOCKET;
        return false;
    }

    s_server_running = true;
#ifdef _WIN32
    s_server_thread = CreateThread(NULL, 0, server_worker, NULL, 0, NULL);
    return (s_server_thread != NULL);
#else
    return (pthread_create(&s_server_thread, NULL, server_worker, NULL) == 0);
#endif
}

void mobcam_web_server_stop(void) {
    if (!s_server_running) return;
    s_server_running = false;

    if (s_listen_sock != INVALID_SOCKET) {
        CLOSE_SOCKET(s_listen_sock);
        s_listen_sock = INVALID_SOCKET;
    }

#ifdef _WIN32
    if (s_server_thread) {
        WaitForSingleObject(s_server_thread, 2000);
        CloseHandle(s_server_thread);
        s_server_thread = NULL;
    }
#else
    if (s_server_thread) {
        pthread_join(s_server_thread, NULL);
        s_server_thread = 0;
    }
#endif
}

void mobcam_web_server_set_callbacks(mobcam_web_query_multi_status_cb query_cb, mobcam_web_apply_ctrl_cb apply_cb) {
    s_query_cb = query_cb;
    s_apply_cb = apply_cb;
}
