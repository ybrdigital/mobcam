# MobCam - Native Camera Plugin for OBS Studio
## Architecture & Developer Specification

This guide describes the plugin architecture, media streaming protocol, build setup, and development procedures for the **MobCam Camera Plugin for OBS Studio**.

---

## 1. Architectural Overview

The MobCam plugin operates as an in-process native 64-bit DLL (`mobcam.dll`) loaded into OBS Studio (`obs64.exe`). It interfaces with the OBS core via `libobs` and processes real-time network packets streamed by the companion MobCam Android application over local Wi-Fi or USB tethering.

```
+-------------------------------------------------------------+
|                 MobCam Android Client                       |
|  - Camera2 / CameraX Hardware Encoder (H.264 / HEVC)        |
|  - 48 kHz AAC Audio Capture                                 |
|  - UDP Auto-Discovery Beacon Broadcast (Port 4750)          |
|  - TCP Control Server (Port 4747)                           |
|  - TCP Video Server (Port 4748)                             |
|  - TCP Audio Server (Port 4749)                             |
+-------------------------------------------------------------+
                               |
                        TCP / UDP Network Link
                               |
+-------------------------------------------------------------+
|                   MobCam OBS Plugin (DLL)                   |
|                                                             |
|  [Discovery Listener]      [Control Client]                 |
|   UDP Listener Port 4750    TCP JSON Handshake Port 4747    |
|                                                             |
|  [Media Client Receivers]                                   |
|   TCP Video Stream Port 4748 -> [H.264 / HEVC Decoder]      |
|   TCP Audio Stream Port 4749 -> [AAC Audio Decoder]         |
|          |                         |                        |
|   libavcodec (FFmpeg)       libavcodec (FFmpeg)             |
|   YUV420 Planar Frames      48 kHz PCM Float Samples        |
|          |                         |                        |
|          v                         v                        |
|   obs_source_output_video   obs_source_output_audio         |
|                                                             |
|  [In-OBS Web Control Server]                                |
|   Embedded HTTP Server (Port 4752 on 127.0.0.1)             |
+-------------------------------------------------------------+
```

---

## 2. Source Code Layout

| Directory / File | Description |
|---|---|
| `plugin/src/plugin-main.c` | OBS plugin entry point (`obs_module_load`, `obs_module_unload`), registers source and servers. |
| `plugin/src/mobcam-source.c` | OBS source callbacks (`create`, `destroy`, `update`, `tick`, `video_render`). |
| `plugin/src/mobcam-source.h` | Core data structures and source interface definition. |
| `plugin/src/net/discovery_listener.c` | Background UDP socket listening for Android device announcements on UDP port 4750. |
| `plugin/src/net/control_client.c` | Handles JSON command dispatch to phone (zoom, flash, exposure, resolution) via TCP 4747. |
| `plugin/src/net/media_client.c` | TCP media socket connection, packet framing, and demuxing (video on 4748, audio on 4749). |
| `plugin/src/decode/h264_decoder.c` | Hardware/FFmpeg H.264/HEVC decoder converting byte streams into YUV420 frames. |
| `plugin/src/decode/aac_decoder.c` | FFmpeg AAC audio decoder outputting synchronized 48 kHz PCM samples. |
| `plugin/src/decode/watermark_overlay.c` | Real-time pixel overlay for watermarking. |
| `plugin/src/ui/mobcam-properties.c` | OBS Properties panel (Device discovery selection, manual IP/port, stream telemetry displays). |
| `plugin/src/ui/web_control_server.c` | Lightweight embedded HTTP server serving quick controls dock at `http://localhost:4752/`. |
| `plugin/src/cjson/` | Embedded cJSON parser for JSON control packets. |
| `plugin/installer/setup.iss` | Inno Setup Windows installer compilation script. |

---

## 3. Communication Protocol & Ports

The MobCam communication pipeline is divided across distinct ports:

| Port | Protocol | Direction | Listening Component | Initiator | Purpose | Configurable |
|---|---|---|---|---|---|---|
| **4750** | UDP | Inbound on PC | OBS Plugin (`discovery_listener.c`) | Android App | Subnet beacon discovery | Yes |
| **4747** | TCP | Outbound from PC | Android App | OBS Plugin (`control_client.c`) | JSON handshake & remote controls | Yes |
| **4748** | TCP | Outbound from PC | Android App | OBS Plugin (`media_client.c`) | H.264 / HEVC video stream | Fixed |
| **4749** | TCP | Outbound from PC | Android App | OBS Plugin (`media_client.c`) | AAC audio stream | Fixed |
| **4752** | HTTP | Loopback (`127.0.0.1`) | OBS Plugin (`web_control_server.c`) | OBS Browser Dock | Quick Controls web interface | Fixed |

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

### 3.2 TCP Control Handshake (Port 4747)
The plugin establishes a direct low-latency TCP control connection to the phone on port `4747` for JSON-formatted commands (torch toggles, zoom levels, exposure values, and connection lifecycle).

### 3.3 TCP Media Streams (Video Port 4748, Audio Port 4749)
Dedicated TCP media connections receive binary framed stream data:
- **Frame Length**: 4 bytes (Big Endian `uint32_t`)
- **Presentation Timestamp (PTS)**: 8 bytes (`int64_t` microseconds)
- **Payload**: Raw H.264/HEVC NAL units on port 4748, or raw AAC audio packets on port 4749.

---

## 4. Building the Plugin

### 4.1 Prerequisites
- Windows 10/11 x64
- Visual Studio 2022 or Visual Studio 2026 with "Desktop development with C++"
- CMake 3.22+
- Inno Setup 6 (optional, for packaging installer)

### 4.2 Building the C Plugin
```powershell
cd plugin
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
The compiled DLL will be generated at `plugin/build/bin/Release/mobcam.dll`.

### 4.3 Packaging the Windows Installer
```powershell
# Using Inno Setup 6 Compiler (adjust path if installed in custom directory)
$iscc = if (Test-Path "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe") {
    "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe"
} else {
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
}
& $iscc plugin\installer\setup.iss
```

---

## 5. Debugging in OBS Studio

1. Install your compiled `mobcam.dll` to `%ProgramData%\obs-studio\plugins\mobcam\bin\64bit\`.
2. Copy `plugin/data` to `%ProgramData%\obs-studio\plugins\mobcam\data\`.
3. Launch OBS Studio in debugging mode.
4. Attach Visual Studio Debugger to `obs64.exe` (`Debug > Attach to Process`).
5. View real-time log messages output via OBS's `blog(LOG_INFO, ...)` in the OBS log window (`Help > Log Files > View Current Log`).

---

## 6. Trademarks & Legal Notice

OBS® and OBS Studio® are registered trademarks of Wizards of OBS LLC. MobCam is an independent open-source project and is not affiliated with, sponsored by, or endorsed by the OBS Project or Wizards of OBS LLC.
