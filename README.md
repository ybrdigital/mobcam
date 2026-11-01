# MobCam - OBS Studio Camera

<div align="center">
  <img src="assets/img/logo.png" alt="MobCam Logo" width="110" height="110" style="border-radius: 24px; box-shadow: 0 8px 30px rgba(0, 230, 118, 0.35);">
  <br><br>
  <h3>Your Android Phone. Your Professional OBS Camera.</h3>
  <p>
    Turn your Android smartphone into a high-quality wireless or USB camera for OBS Studio. Stream video and audio directly into OBS with low latency and professional camera controls.
  </p>
  <p>
    <a href="https://ybrdigital.github.io/mobcam/"><img src="https://img.shields.io/badge/Website-GitHub%20Pages-00E676?style=for-the-badge&logo=github&logoColor=black" alt="Live Website"></a>
    <a href="assets/downloads/v1.0.0/MobCam-OBS-Plugin-v1.0.0-Setup.exe"><img src="https://img.shields.io/badge/Download%20Plugin-v1.0.0%20(Win64)-0052CC?style=for-the-badge&logo=windows&logoColor=white" alt="Download Windows Plugin"></a>
    <a href="https://play.google.com/store/apps/details?id=com.ybrdigital.mobcam"><img src="https://img.shields.io/badge/Google%20Play-Get%20App-34A853?style=for-the-badge&logo=google-play&logoColor=white" alt="Google Play"></a>
    <a href="LICENSE"><img src="https://img.shields.io/badge/License-GPL%202.0%2B-blue?style=for-the-badge" alt="GPL 2.0 or later"></a>
  </p>
</div>

---

## 🌐 Official Website & Live Documentation

- **Live Landing Page**: **[https://ybrdigital.github.io/mobcam/](https://ybrdigital.github.io/mobcam/)**
- **GitHub Repository**: **[https://github.com/ybrdigital/mobcam](https://github.com/ybrdigital/mobcam)**

---

## 🚀 Key Features

1. **4K UHD Video Streaming**: Broadcast in full 1080p and pristine 4K UHD resolutions directly into your OBS scenes.
2. **Up to 60 FPS**: Silky-smooth high-framerate video capture for live streaming, esports, tutorials, and content creation.
3. **Ultra-Low Latency Mode**: Optimized peer-to-peer transmission pipeline engineered for real-time responsiveness over Wi-Fi and wired USB.
4. **Wireless Wi-Fi Streaming**: Wire-free freedom across 5 GHz local networks with automatic UDP device discovery on port `4748`.
5. **USB Tethering Support**: Plug-and-play wired connectivity via standard Android USB tethering for zero-jitter, packet-loss-free broadcasts.
6. **Integrated OBS Source**: Native `libobs` source module (`MobCam Source`) appearing directly in the OBS Studio Sources list.
7. **Built-in Camera Controls**: Embedded OBS browser dock (`http://localhost:4752/` via `Docks > MobCam Controls`) to manage zoom presets, flash, exposure compensation, and autofocus remotely.
8. **Synchronized 48 kHz Audio**: Crystal-clear mobile microphone capture mapped into the OBS Audio Mixer with timestamp sync.
9. **Hardware-Accelerated Encoding**: Direct hardware encoding on Android (MediaCodec H.264/HEVC) and hardware/FFmpeg decoding (`libavcodec`) in OBS.

---

## 📂 Repository Structure

```
mobcam/
├── index.html                # Modern product landing page (GitHub Pages)
├── privacy-policy.html       # Official Privacy Policy
├── terms-of-service.html     # Official Terms of Service
├── LICENSE                   # GNU General Public License v2.0
├── THIRD_PARTY_LICENSES.md   # Third-party licensing audit (libobs, FFmpeg, cJSON)
├── CONTRIBUTING.md           # Guidelines for contributing to the plugin
├── SECURITY.md               # Security vulnerability reporting policy
├── CHANGELOG.md              # Version changelog
├── README.md                 # Repository documentation (this file)
│
├── assets/
│   ├── downloads/            # Verified release binaries & checksums
│   │   └── v1.0.0/           # v1.0.0 Windows Installer (Setup.exe) & SHA256
│   ├── img/                  # Logos, icons, hero product artwork
│   ├── screenshots/          # Real Android app & OBS integration screenshots
│   └── version.json          # Central product & release metadata
│
├── css/
│   └── style.css             # Premium responsive SaaS theme stylesheet
│
├── js/
│   └── main.js               # Dynamic version loader, GitHub API & UI scripts
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
│   ├── installation.md       # Step-by-step installation instructions
│   ├── troubleshooting.md    # Diagnostics for connectivity, latency & audio
│   └── development.md        # Architecture, protocol specification & build details
│
└── .github/
    └── workflows/
        └── build-plugin.yml  # GitHub Actions Windows x64 CI build workflow
```

---

## 📦 Downloads & Releases

### Latest Release: v1.0.0 (Stable)

| Artifact | Platform | Architecture | Compatibility | Size | Checksum (SHA-256) |
|---|---|---|---|---|---|
| **[Windows Installer](assets/downloads/v1.0.0/MobCam-OBS-Plugin-v1.0.0-Setup.exe)** | Windows 10 / 11 | x64 (64-bit) | OBS Studio 28.0 - 31.0+ | 2.1 MB | `A33CD424E25D638B5BA3920264BE76C270F2B61FD7E0014817AEA5D5F84BFEEB` |
| **[Android App (Google Play)](https://play.google.com/store/apps/details?id=com.ybrdigital.mobcam)** | Android 8.0+ | arm64-v8a / armeabi-v7a | Camera2 / CameraX | — | Official Google Play Release |

> **Note**: The Windows setup binary is verified and bundled under `assets/downloads/v1.0.0/`. When published via GitHub Releases, assets are automatically linked through the central version configuration.

---

## 🛠️ Quick Installation Guide

1. **Download the Windows Installer**: Grab `MobCam-OBS-Plugin-v1.0.0-Setup.exe`.
2. **Close OBS Studio**: Ensure `obs64.exe` is closed before running the setup wizard.
3. **Run the Installer**: Follow the wizard prompts to install the plugin into `%ProgramData%\obs-studio\plugins\mobcam\`.
4. **Open OBS Studio**: Launch OBS Studio on your PC.
5. **Add MobCam Source**: In the **Sources** dock, click **+** &rarr; **MobCam Source**.
6. **Connect your Android Device**:
   - **Via USB (Lowest Latency)**: Connect USB cable, enable **USB Tethering** in phone settings, tap Start Stream in MobCam, and click OK in OBS Studio.
   - **Via Wi-Fi**: Ensure phone and PC are on the same Wi-Fi network, start stream in MobCam, select your phone from auto-discovery (or enter the IP manually), and click OK.

For complete setup instructions, see [docs/installation.md](docs/installation.md).  
For troubleshooting common issues, see [docs/troubleshooting.md](docs/troubleshooting.md).

---

## 💻 Building the Plugin from Source

The MobCam OBS plugin is written in C and builds using CMake and Visual Studio 2022 on Windows (x64).

```powershell
# 1. Clone repository
git clone https://github.com/ybrdigital/mobcam.git
cd mobcam/plugin

# 2. Configure with CMake
cmake -B build -S .

# 3. Compile Release DLL
cmake --build build --config Release
```

The resulting binary will be located at:
`build/bin/Release/mobcam.dll`

For full architecture and protocol details, see [docs/development.md](docs/development.md).

---

## 📜 Licensing & Open Source

- **OBS Studio Plugin**: Licensed under the **GNU General Public License v2.0 or later** ([GPL-2.0-or-later](LICENSE)).
- **Third-Party Dependencies**: See [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) for audits of `libobs` (GPLv2+), FFmpeg (LGPLv2.1+/GPLv2+ dynamic linking), and `cJSON` (MIT).
- **Android Mobile App & Backend**: The **MobCam Android Mobile App** distributed via Google Play and backend infrastructure remain proprietary products of YBR Digital and are not included in this open-source repository.

---

## 🤝 Contributing

Contributions, bug reports, and suggestions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) and [SECURITY.md](SECURITY.md) before submitting pull requests or vulnerability reports.

---

## 📬 Support & Contact

- **Brand**: YBR Digital
- **Developer**: Yash Rayjada
- **Support Email**: [info@ybrdigital.in](mailto:info@ybrdigital.in)
- **Official Website**: [https://www.ybrdigital.in](https://www.ybrdigital.in)
- **Privacy Policy**: [https://ybrdigital.github.io/mobcam/privacy-policy.html](privacy-policy.html)
- **Terms of Service**: [https://ybrdigital.github.io/mobcam/terms-of-service.html](terms-of-service.html)
