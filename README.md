# MobCam - Native Camera Plugin for OBS Studio

<div align="center">
  <img src="assets/img/logo.png" alt="MobCam Logo" width="110" height="110" style="border-radius: 24px; box-shadow: 0 8px 30px rgba(0, 230, 118, 0.35);">
  <br><br>
  <h3>Your Android Phone. Your Professional Camera.</h3>
  <p>
    A native Windows camera plugin that connects MobCam Android to OBS Studio over Wi-Fi and USB. Stream high-definition video and audio directly into your broadcast scenes with ultra-low latency and remote camera controls.
  </p>
  <p>
    <a href="https://ybrdigital.github.io/mobcam/"><img src="https://img.shields.io/badge/Website-GitHub%20Pages-00E676?style=for-the-badge&logo=github&logoColor=black" alt="Official Website"></a>
    <a href="https://github.com/ybrdigital/mobcam/releases"><img src="https://img.shields.io/badge/Releases-v1.0.0%20(Win64)-0052CC?style=for-the-badge&logo=windows&logoColor=white" alt="Official Releases"></a>
    <a href="https://play.google.com/store/apps/details?id=com.ybrdigital.mobcam"><img src="https://img.shields.io/badge/Google%20Play-Get%20App-34A853?style=for-the-badge&logo=google-play&logoColor=white" alt="Google Play App"></a>
    <a href="LICENSE"><img src="https://img.shields.io/badge/License-GPL%202.0%2B-blue?style=for-the-badge" alt="GPL-2.0-or-later"></a>
  </p>
</div>

---

## Overview

**MobCam** is an independent third-party open-source plugin compatible with **OBS Studio** on Windows (64-bit). It enables live streamers, content creators, and broadcast professionals to use their Android smartphone camera as a high-fidelity video capture device without requiring capture cards, complex virtual camera drivers, or cloud streaming services.

- **Official Website & Live Documentation**: [https://ybrdigital.github.io/mobcam/](https://ybrdigital.github.io/mobcam/)
- **Official GitHub Repository**: [https://github.com/ybrdigital/mobcam](https://github.com/ybrdigital/mobcam)
- **Official Releases**: [https://github.com/ybrdigital/mobcam/releases](https://github.com/ybrdigital/mobcam/releases)

---

## Key Features

1. **High-Definition Video Streaming**: Broadcast in pristine 1080p Full HD and 4K UHD resolutions directly into OBS Studio.
2. **Up to 60 FPS Capture**: High-framerate capture engineered for fast-paced gameplay streaming, tutorials, and multicam productions.
3. **Ultra-Low Latency Pipeline**: Direct peer-to-peer frame delivery over local Wi-Fi or wired USB.
4. **Automatic Subnet Discovery**: Background UDP beacon detection on port `4750` automatically identifies active MobCam Android devices on your local network.
5. **Plug-and-Play USB Tethering**: Zero-jitter wired connectivity using standard Android USB tethering for environments where Wi-Fi is congested or unavailable.
6. **Native OBS Source Integration**: Appears natively under **Sources > MobCam Source** using the official `libobs` module API.
7. **Embedded In-OBS Quick Controls Dock**: Lightweight HTTP web dock (`http://localhost:4752/` via `Docks > MobCam Controls`) to manage zoom presets (1x, 1.5x, 2x), autofocus modes, flashlight, and exposure compensation remotely.
8. **Synchronized 48 kHz Stereo Audio**: High-clarity mobile microphone capture delivered directly into the OBS Audio Mixer with presentation timestamp (PTS) synchronization.
9. **Hardware-Accelerated Decoding**: High-performance H.264 / HEVC video decoding powered by `libavcodec` (FFmpeg) with YUV420 planar frame rendering.

---

## System Compatibility

| Component | Requirement |
|---|---|
| **Operating System** | Windows 10 (64-bit) / Windows 11 (64-bit) |
| **OBS Studio** | OBS Studio 28.0 - 31.0+ (native 64-bit x64) |
| **Android Client** | Android 8.0 Oreo (API 26) or newer with Camera2 API support |
| **Connectivity** | 5 GHz Wi-Fi or USB connection (Android USB Tethering) |

---

## Network Architecture & Ports

MobCam operates purely on a peer-to-peer (P2P) local network architecture. Zero video or audio data is ever routed through external servers or cloud services.

| Port | Protocol | Direction | Listening Component | Initiator | Purpose |
|---|---|---|---|---|---|
| **4750** | UDP | Inbound on PC | OBS Plugin (`discovery_listener.c`) | Android App | Local subnet device beacon auto-discovery |
| **4747** | TCP | Outbound from PC | Android App | OBS Plugin (`control_client.c`) | Control handshake and camera settings |
| **4748** | TCP | Outbound from PC | Android App | OBS Plugin (`media_client.c`) | H.264 / HEVC video media stream |
| **4749** | TCP | Outbound from PC | Android App | OBS Plugin (`media_client.c`) | AAC audio media stream |
| **4752** | HTTP | Loopback (`127.0.0.1`) | OBS Plugin (`web_control_server.c`) | OBS Browser Dock | Local Quick Controls dock web interface |

### Windows Firewall Configuration
If Windows Defender Firewall prompts you, allow incoming UDP discovery on port `4750` and outgoing TCP connections on ports `4747-4749`. See [docs/troubleshooting.md](docs/troubleshooting.md) for automated PowerShell firewall configuration commands.

---

## Downloads & Releases

Official binary downloads are published via [GitHub Releases](https://github.com/ybrdigital/mobcam/releases).

### Latest Stable Release: v1.0.0

| Package | Platform | Architecture | Distribution Link |
|---|---|---|---|
| **Windows Installer** | Windows 10 / 11 | x64 (64-bit) | [MobCam-OBS-Plugin-v1.0.0-Setup.exe](https://github.com/ybrdigital/mobcam/releases/latest/download/MobCam-OBS-Plugin-v1.0.0-Setup.exe) |
| **Android Application** | Android 8.0+ | arm64-v8a / armeabi-v7a | [Google Play Store Listing](https://play.google.com/store/apps/details?id=com.ybrdigital.mobcam) |

*Local fallback installer binary and verification metadata are preserved under [`assets/downloads/v1.0.0/`](assets/downloads/v1.0.0/).*

---

## Installation & Quick Start

1. **Install the Windows Plugin**: Download and run `MobCam-OBS-Plugin-v1.0.0-Setup.exe` (ensure OBS Studio is closed during installation).
2. **Install the Android App**: Install MobCam from Google Play on your smartphone.
3. **Launch OBS Studio**: Open OBS Studio on your PC.
4. **Add the Source**: In the **Sources** dock, click **+** &rarr; **MobCam Source**.
5. **Connect**:
   - **Wired USB (Recommended for lowest latency)**: Connect phone via USB cable, enable **USB Tethering** in phone settings, tap **Start Stream** in MobCam, and click **OK** in OBS.
   - **Wireless Wi-Fi**: Connect phone and PC to the same 5 GHz Wi-Fi network, tap **Start Stream** in MobCam, select your phone from the auto-detect dropdown, and click **OK**.

For full step-by-step instructions and manual ZIP installation for portable OBS configurations, see [docs/installation.md](docs/installation.md).

---

## Building from Source

The MobCam OBS Studio plugin is written in C and builds using CMake and Visual Studio on Windows x64.

### Build Requirements
- Windows 10/11 x64
- Visual Studio 2022 or Visual Studio 2026 (with "Desktop development with C++")
- CMake 3.22 or newer

### Build Instructions
```powershell
# 1. Clone the repository
git clone https://github.com/ybrdigital/mobcam.git
cd mobcam/plugin

# 2. Configure CMake in Release mode
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# 3. Build the plugin DLL
cmake --build build --config Release
```
The compiled DLL will be generated at `plugin/build/bin/Release/mobcam.dll`.

For detailed architecture diagrams, decoder pipelines, and debugging steps, consult [docs/development.md](docs/development.md).

---

## Repository Structure

```
mobcam/
├── index.html                # Official product website (GitHub Pages)
├── privacy-policy.html       # Official Privacy Policy
├── terms-of-service.html     # Official Terms of Service
├── LICENSE                   # GNU General Public License v2.0
├── THIRD_PARTY_LICENSES.md   # Third-party licensing audit (libobs, FFmpeg, cJSON)
├── CONTRIBUTING.md           # Maintenance policy & forking guidelines
├── SECURITY.md               # Security vulnerability reporting policy
├── CHANGELOG.md              # Version history and changelog
├── README.md                 # Project documentation (this file)
│
├── assets/
│   ├── downloads/            # Verified release binaries & checksums
│   │   └── v1.0.0/           # Windows installer binary & SHA256SUMS.txt
│   ├── img/                  # Logos, icons, artwork
│   ├── screenshots/          # Real product screenshots
│   └── version.json          # Central product and version metadata
│
├── css/
│   └── style.css             # Responsive theme stylesheet
│
├── js/
│   └── main.js               # Dynamic version loader, GitHub API & UI script
│
├── plugin/                   # Native C OBS Studio Plugin source code
│   ├── CMakeLists.txt        # CMake build configuration
│   ├── README.md             # Plugin developer guide
│   ├── src/                  # Source files (libobs source, decoders, networking)
│   ├── data/                 # Localization & icon resources
│   ├── installer/            # Inno Setup Windows installer script (setup.iss)
│   └── deps/                 # libobs & FFmpeg header definitions & import libs
│
├── docs/                     # Technical documentation
│   ├── installation.md       # Installation guide & portable OBS instructions
│   ├── troubleshooting.md    # Diagnostics for connectivity, latency & audio
│   └── development.md        # Architecture, protocol specification & build details
│
└── .github/
    └── workflows/
        └── build-plugin.yml  # GitHub Actions Windows x64 CI build verification
```

---

## Licensing & Component Separation

### Public Open-Source Component
The **MobCam OBS Studio Plugin** (native C source code, build scripts, and covered binaries) is free software licensed under the **GNU General Public License v2.0 or later** ([GPL-2.0-or-later](LICENSE)). You are free to inspect, modify, and redistribute this software in accordance with the GPL.

### Private Proprietary Components
The **MobCam Android Mobile Application** distributed through the Google Play Store is a proprietary commercial product of **YBR Digital** and is **not** licensed under the GPL. The plugin communicates with the Android application over an independent local TCP/UDP network protocol; this communication across independent network sockets does not subject the proprietary mobile app to the GPL.

### Brand Assets & Identity
The "MobCam" name, logo artwork, icons, marketing imagery, and website styling remain the exclusive proprietary property and copyrighted works of **YBR Digital** and are not licensed under the GPL. Independent forks or derivative distributions must not use official MobCam branding in a manner that implies endorsement or official affiliation.

---

## Maintenance Policy

MobCam is an independently maintained project developed and released by **YBR Digital**.

- Official development, releases, and updates are managed directly by the maintainer.
- External pull requests and collaborative code development are currently not accepted.
- Bug reports, compatibility questions, and feature suggestions are welcome via [GitHub Issues](https://github.com/ybrdigital/mobcam/issues).
- Independent forks and modifications are fully permitted under the GPL-2.0-or-later, but independent forks are not official MobCam releases.

For more information, see [CONTRIBUTING.md](CONTRIBUTING.md).

---

## Security Vulnerability Reporting

If you discover a security vulnerability or sensitive defect, please review our [SECURITY.md](SECURITY.md) policy and email **[info@ybrdigital.in](mailto:info@ybrdigital.in)** directly with diagnostic details.

---

## Support & Contact

- **Developer / Maintainer**: Yash Rayjada (YBR Digital)
- **Technical Support**: [info@ybrdigital.in](mailto:info@ybrdigital.in)
- **Official Website**: [https://ybrdigital.github.io/mobcam/](https://ybrdigital.github.io/mobcam/)
- **Privacy Policy**: [https://ybrdigital.github.io/mobcam/privacy-policy.html](privacy-policy.html)
- **Terms of Service**: [https://ybrdigital.github.io/mobcam/terms-of-service.html](terms-of-service.html)

---

## Third-Party Trademark Disclaimer

OBS® and OBS Studio® are registered trademarks of Wizards of OBS LLC. MobCam is an independent open-source project and is not affiliated with, sponsored by, or endorsed by the OBS Project or Wizards of OBS LLC.
