# MobCam OBS Studio Plugin (`mobcam-obs-plugin`)

[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg)](../LICENSE)
[![Platform: Windows 64-bit](https://img.shields.io/badge/Platform-Windows%2064--bit-0052CC.svg?logo=windows&logoColor=white)](#)
[![OBS Studio: 28.0 - 31.0+](https://img.shields.io/badge/OBS%20Studio-28.0%20--%2031.0%2B-black.svg?logo=obs-studio&logoColor=white)](#)

Native OBS Studio source plugin for **MobCam**, capturing hardware-accelerated H.264 video and 48 kHz AAC audio directly from an Android smartphone over Wi-Fi or USB.

---

## Key Features

- **Native OBS Source**: Registers `mobcam_source` (`MobCam Source`) in OBS Studio.
- **Ultra-Low Latency**: Direct TCP socket streaming, zero intermediate cloud relays.
- **FFmpeg Decoding**: Hardware and software decoding via OBS's native `libavcodec` and `libavutil`.
- **Integrated OBS Quick Controls**: Embedded HTTP dock on port `4752` (`Docks > MobCam Controls`) to manage zoom presets, flash, exposure, and focus directly inside OBS Studio.
- **Auto 16:9 Landscape Lock**: Enforces horizontal widescreen framing to eliminate vertical black pillarboxes.
- **Audio Output**: Exposes a dedicated synchronized audio track into the OBS Audio Mixer.
- **UDP Auto-Discovery**: Automatic detection of active MobCam devices across the local subnet on UDP port 4750.
- **Zero-Configuration Instant Connect**: Seamless, one-click connection with automatic stream negotiation.

---

## Directory Structure

```
plugin/
├── CMakeLists.txt          # Root CMake configuration
├── README.md               # Plugin developer documentation
├── src/                    # Complete C source code
│   ├── plugin-main.c       # OBS module entry point
│   ├── mobcam-source.c/h   # Source implementation & video rendering
│   ├── decode/             # H.264 and AAC decoders using libavcodec
│   ├── net/                # Discovery listener, media socket, control client
│   ├── ui/                 # Source properties & embedded web server dock
│   └── cjson/              # cJSON library (MIT)
├── data/                   # Plugin data files (locale strings, logo)
│   ├── locale/en-US.ini
│   └── mobcam-logo.png
├── installer/              # Inno Setup Windows installer script (setup.iss)
└── deps/                   # libobs headers, FFmpeg headers, import libraries
```

---

## Building from Source

### Prerequisites
- Windows 10 / Windows 11 (64-bit)
- CMake 3.22+
- Microsoft Visual Studio 2022 (MSVC v143 or newer) with C++ Desktop Workload

### Steps

```powershell
# 1. Navigate to the plugin folder
cd plugin

# 2. Configure CMake
cmake -B build -S .

# 3. Build Release DLL
cmake --build build --config Release
```

The compiled binary will be located at:
`build/bin/Release/mobcam.dll`

---

## Manual Installation

To install the built plugin into your local OBS Studio:

1. Copy `build/bin/Release/mobcam.dll` to:
   ```text
   %ProgramData%\obs-studio\plugins\mobcam\bin\64bit\mobcam.dll
   ```
2. Copy `data/*` to:
   ```text
   %ProgramData%\obs-studio\plugins\mobcam\data\
   ```
3. Restart OBS Studio.

---

## License

This plugin is licensed under the **GNU General Public License v2.0 or later** (`GPL-2.0-or-later`).  
See [LICENSE](../LICENSE) for details.

---

## Trademark Notice

OBS® and OBS Studio® are registered trademarks of Wizards of OBS LLC. MobCam is an independent open-source project and is not affiliated with, sponsored by, or endorsed by the OBS Project or Wizards of OBS LLC.
