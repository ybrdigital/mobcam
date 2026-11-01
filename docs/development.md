# MobCam OBS Studio Plugin - Developer Documentation & Architecture

This guide describes the plugin architecture, media streaming protocol, build setup, and development procedures for the **MobCam OBS Studio Plugin**.

---

## 1. Architectural Overview

The MobCam plugin operates as an in-process native DLL loaded into OBS Studio (`obs64.exe`). It interfaces with the OBS core via `libobs` and processes real-time network packets streamed by the MobCam Android application.

```
+-------------------------------------------------------------+
|                 MobCam Android Client                       |
|  - Camera2 / CameraX Hardware Encoder (H.264 / HEVC)        |
|  - 48 kHz AAC Audio Capture                                 |
|  - UDP Auto-Discovery Beacon (Port 4748)                    |
|  - TCP Media Server (Port 4747)                             |
+-------------------------------------------------------------+
                               |
                       TCP / UDP Network Link
                               |
+-------------------------------------------------------------+
|                   MobCam OBS Plugin (DLL)                   |
|                                                             |
|  [Discovery Listener]      [Control Client]                 |
|   UDP Broadcast Port 4748   JSON Handshake & Commands       |
|                                                             |
|  [Media Client Receiver]                                    |
|   TCP Stream Reader -> Packet Demuxer                       |
|          |                         |                        |
|   [H.264 Video Decoder]     [AAC Audio Decoder]             |
|   libavcodec (FFmpeg)       libavcodec (FFmpeg)             |
|   YUV420 Planar Frames      48 kHz PCM Float Samples        |
|          |                         |                        |
|          v                         v                        |
|   obs_source_output_video   obs_source_output_audio         |
|                                                             |
|  [In-OBS Web Control Server]                                |
|   Embedded HTTP Server (Port 4752)                          |
+-------------------------------------------------------------+
```

---

## 2. Source Code Layout

| Directory / File | Description |
|---|---|
| `plugin/src/plugin-main.c` | OBS plugin entry point (`obs_module_load`, `obs_module_unload`), registers source and servers. |
| `plugin/src/mobcam-source.c` | OBS source callbacks (`create`, `destroy`, `update`, `tick`, `video_render`). |
| `plugin/src/mobcam-source.h` | Core data structures and source interface definition. |
| `plugin/src/net/discovery_listener.c` | Background UDP socket listening for Android device announcements on port 4748. |
| `plugin/src/net/control_client.c` | Handles JSON command dispatch to phone (zoom, flash, exposure, resolution). |
| `plugin/src/net/media_client.c` | TCP media socket connection, packet framing, and demuxing. |
| `plugin/src/decode/h264_decoder.c` | Hardware/FFmpeg H.264 decoder converting byte streams into YUV420 frames. |
| `plugin/src/decode/aac_decoder.c` | FFmpeg AAC audio decoder outputting synchronized 48 kHz PCM samples. |
| `plugin/src/decode/watermark_overlay.c` | Real-time pixel overlay for watermarking. |
| `plugin/src/ui/mobcam-properties.c` | OBS Properties panel (Device discovery selection, manual IP/port, stream telemetry displays). |
| `plugin/src/ui/web_control_server.c` | Lightweight embedded HTTP server serving quick controls dock at port 4752. |
| `plugin/src/cjson/` | Embedded cJSON parser for JSON control packets. |
| `plugin/installer/setup.iss` | Inno Setup Windows installer compilation script. |

---

## 3. Communication Protocol

### 3.1 UDP Auto-Discovery (Port 4750)
The Android device periodically broadcasts a UDP JSON beacon announcing device availability:
```json
{
  "service": "mobcam",
  "version": "1.0.0",
  "deviceName": "Pixel 8 Pro",
  "ip": "192.168.1.45",
  "port": 4747,
  "httpPort": 4752
}
```
The plugin's discovery thread listens on UDP port `4750`, parses the beacon, and populates the OBS source property dropdown.

### 3.2 TCP Media Stream Framing (Port 4747)
The plugin establishes a direct low-latency TCP connection to the phone on port `4747`. Seamless streaming starts immediately with zero manual configuration.
Each subsequent binary packet follows a compact header framing:
- **Frame Length**: 4 bytes (Big Endian `uint32_t`)
- **Presentation Timestamp (PTS)**: 8 bytes (`int64_t` microseconds)
- **Payload**: Raw H.264 NAL units (SPS/PPS/IDR/P-frames) or raw AAC audio frames

---

## 4. Building the Plugin

### 4.1 Prerequisites
- Windows 10/11 x64
- Visual Studio 2022 with Desktop development with C++
- CMake 3.22+

### 4.2 Building
```powershell
cd plugin
cmake -B build -S .
cmake --build build --config Release
```

### 4.3 Packaging the Installer
```powershell
& "C:\Users\Yash\AppData\Local\Programs\Inno Setup 6\ISCC.exe" plugin\installer\setup.iss
```

---

## 5. Debugging in OBS Studio

1. Install your compiled `mobcam.dll` to `%ProgramData%\obs-studio\plugins\mobcam\bin\64bit\`.
2. Launch OBS Studio in portable or debugging mode.
3. Attach Visual Studio Debugger to `obs64.exe` (`Debug > Attach to Process`).
4. Set breakpoints in `mobcam-source.c` or `h264_decoder.c`.
5. View real-time log messages output via OBS's `blog(LOG_INFO, ...)` in the OBS log window.
