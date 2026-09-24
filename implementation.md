# Implementation & Architecture Guide: MobCam - OBS Camera Website & Plugin Archive

**Product**: MobCam - OBS Camera  
**Developed by**: Yash Rayjada  
**Support Email**: `info@ybrdigital.in`  
**GitHub Repository**: [https://github.com/ybrdigital/mobcam](https://github.com/ybrdigital/mobcam)  
**Live Website URL**: [https://ybrdigital.github.io/mobcam/](https://ybrdigital.github.io/mobcam/)  
**Target Platform**: Android (Mobile App via Google Play) & Windows 10/11 64-bit (OBS Studio 28.0 - 31.0+ Plugin)  

---

## 1. Executive Summary

This document details the production architecture, release management lifecycle, and frontend implementation for the official website of **MobCam - OBS Camera**. The website serves as the primary distribution hub for:
1. **The OBS Studio Plugin Installer**: Direct Windows `.exe` setup package with cryptographic verification (SHA-256).
2. **Multi-Version Release Archive**: A future-proof version management architecture allowing users to browse, download, and review release notes for any current or historical version of the OBS plugin.
3. **The Android Mobile App**: Direct links to the Google Play Store (`com.ybrdigital.mobcam`) where the mobile client is exclusively distributed.
4. **Comprehensive Documentation**: Interactive step-by-step setup guides (Wi-Fi, USB Tethering, OBS Web Dock), technical specifications, complete feature capabilities catalog, and legal policies (Privacy Policy & Terms of Service).

---

## 2. Multi-Version Plugin Management Architecture

To support continuous updates and maintain backwards compatibility, the website implements a structured, data-driven release catalog.

### 2.1 Release Data Schema (`releases` in `main.js`)
All plugin releases are defined in a structured array of version objects. When a new version is released in the future, simply prepend a new version object into this catalog.

```javascript
{
  version: "1.0.0",
  tag: "v1.0.0",
  isLatest: true,
  releaseDate: "September 2026",
  obsCompatibility: "OBS Studio 28.0 - 31.0+ (64-bit)",
  windowsCompatibility: "Windows 10 / 11 (64-bit)",
  installerPath: "assets/downloads/v1.0.0/MobCam-OBS-Plugin-v1.0.0-Setup.exe",
  installerSize: "2.1 MB",
  sha256: "19501575BAE2017A5F1E6FBACAE27838ADD7AAA55A66EFF548BA09D22545449D",
  changelog: [
    "Production-ready Windows 64-bit installer for OBS Studio 28.0 - 31.0+.",
    "Ultra-low latency streaming engine (<30ms wired USB, <50ms 5GHz Wi-Fi).",
    "Hardware-accelerated YUV420 color rendering pipeline with zero chroma ghosting.",
    "Integrated OBS Quick Controls web dock hosted natively on http://localhost:4752/.",
    "Auto 16:9 landscape orientation lock prevents vertical pillarboxing.",
    "Dynamic network reconnect: automatic IP recovery upon router handoffs.",
    "Centered clean white watermark in OBS with pure transparency.",
    "Isolated peer-to-peer TCP transmission without third-party cloud servers."
  ]
}
```

### 2.2 Storage & Directory Layout
Each release version is stored within its own versioned subdirectory in `assets/downloads/<version>/`:
```
assets/
└── downloads/
    ├── v1.0.0/
    │   ├── MobCam-OBS-Plugin-v1.0.0-Setup.exe
    │   └── SHA256SUMS.txt
    ├── v1.0.1/ (future)
    └── v1.1.0/ (future)
```

### 2.3 UI Capabilities for Version Management
- **Hero & Primary CTA**: Always targets the latest stable release (`v1.0.0`) with instant one-click download.
- **Dedicated Releases Archive Section (`#releases`)**:
  - Highlights the **Latest Release** with detailed badges, installation paths, file size, direct setup download, and one-click SHA-256 hash copying (rendered in high-contrast white `#FFFFFF`).
  - Lists **All Historical Releases** with clean bullet-point release notes (no cluttering category tags).

---

## 3. Technical Specifications & Protocols

### 3.1 Android Mobile Application
- **Distribution**: Exclusively via Google Play Store (`com.ybrdigital.mobcam`).
- **Minimum OS**: Android 8.0 Oreo (API Level 26).
- **Recommended OS**: Android 10+ (API Level 29+) with Camera2 HAL3 / CameraX 1.4.
- **Video Compression**: Hardware MediaCodec AVC (H.264), HEVC (H.265), MJPEG fallback.
- **Resolutions**: 720p HD, 1080p Full HD, 4K UHD (2160p).
- **Framerate**: 30 FPS standard, 60 FPS ultra-smooth.
- **Audio Stream**: 48 kHz stereo AAC audio with timestamp synchronizer.
- **Orientation Control**: Intelligent auto 16:9 landscape locking upon stream initialization.
- **Network Ports**:
  - `4747 TCP/HTTP`: Video & audio streaming server and REST control endpoint.
  - `4752 HTTP`: OBS Web Controls dock server.

### 3.2 OBS Studio Plugin (Windows)
- **Host OS**: Windows 10 (64-bit) or Windows 11 (64-bit).
- **OBS Studio Compatibility**: OBS Studio 28.0, 29.0, 30.0, 31.0+ (64-bit).
- **Installation Method**:
  - Automated NSIS Installer (`MobCam-OBS-Plugin-v1.0.0-Setup.exe`) auto-detecting `%ProgramFiles%\obs-studio\obs-plugins\64bit` and `%APPDATA%\obs-studio\plugins`.
- **Integrated Browser Dock**: OBS menu `Docks` -> `MobCam Controls` targeting `http://localhost:4752/` for zero-friction remote control of phone flash, zoom, focus, and exposure.

---

## 4. Legal Compliance & Privacy Framework

- **Privacy Policy**:
  - **Zero Data Collection**: No user tracking, no personal information collection, and no analytics SDKs.
  - **100% Peer-to-Peer**: Video and audio frames are streamed solely over the local Wi-Fi or USB tethered network between the user's Android device and Windows PC.
  - **No Cloud Servers**: Frames are never uploaded, stored, or processed on external cloud infrastructure.
  - **Camera & Microphone Permissions**: Strictly utilized in real-time to generate the OBS input source.
- **Terms of Service**:
  - **License**: End-user license for personal and professional broadcasting.
  - **Distribution**: All app distribution handled securely through Google Play.
  - **Support Contact**: `info@ybrdigital.in` managed by Yash Rayjada.

---

## 5. Deployment & GitHub Pages Setup

1. **Repository**: `https://github.com/ybrdigital/mobcam` (branch: `main`).
2. **Static Asset Pipeline**: Clean vanilla HTML5, CSS3, Bootstrap 5.3.3, jQuery 3.7.1, and FontAwesome 6 icons.
3. **`.nojekyll` Marker**: Included in repository root to ensure all directories and files are served intact without Jekyll build interference.
4. **Live URL**: `https://ybrdigital.github.io/mobcam/`
