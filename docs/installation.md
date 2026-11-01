# MobCam - Native Camera Plugin for OBS Studio
## Installation & Setup Guide

This guide walks you through installing and configuring the **MobCam Camera Plugin for OBS Studio** on Windows 10/11 and connecting it to your Android device running MobCam over Wi-Fi or USB tethering.

---

## 1. System Requirements

### Windows PC
- **Operating System**: Windows 10 or Windows 11 (64-bit)
- **Host Application**: OBS Studio 28.0 - 31.0+ (64-bit native)
- **Network Interfaces**: Local Wi-Fi (5 GHz strongly recommended) or USB connection (Android USB tethering)

### Android Device
- **Operating System**: Android 8.0 Oreo (API Level 26) or newer (Android 10+ recommended)
- **Hardware**: Camera with Camera2 API / HAL3 support
- **Application**: [MobCam on Google Play Store](https://play.google.com/store/apps/details?id=com.ybrdigital.mobcam)

---

## 2. Installation Options

### Option A: Windows Installer (Recommended)

1. **Download the Official Installer**:
   Download `MobCam-OBS-Plugin-v1.0.0-Setup.exe` from the official [Releases page](https://github.com/ybrdigital/mobcam/releases) or the [official landing page](https://ybrdigital.github.io/mobcam/).

2. **Close OBS Studio**:
   Ensure OBS Studio (`obs64.exe`) is completely closed before running the setup wizard so shared DLL files can be updated cleanly.

3. **Run the Installer**:
   Double-click `MobCam-OBS-Plugin-v1.0.0-Setup.exe` and follow the prompts. The installer will:
   - Verify that OBS Studio is installed.
   - Present the GNU General Public License v2.0 or later terms.
   - Install `mobcam.dll` into `%ProgramData%\obs-studio\plugins\mobcam\bin\64bit\`.
   - Install locale and icon resources into `%ProgramData%\obs-studio\plugins\mobcam\data\`.
   - Auto-configure the MobCam Controls browser dock in your local OBS configuration (`%APPDATA%\obs-studio\user.ini`).
   - Register a clean uninstaller in Windows Add/Remove Programs.

4. **Launch OBS Studio**:
   Open OBS Studio. You will now see **MobCam Source** available in the Sources list (`+ > MobCam Source`).

---

### Option B: Manual Installation (Standard or Portable OBS Studio)

If you are using a portable version of OBS Studio or prefer manual file placement:

#### Standard OBS Plugin Location:
1. Create the plugin destination folders:
   ```powershell
   New-Item -ItemType Directory -Force "$env:ProgramData\obs-studio\plugins\mobcam\bin\64bit"
   New-Item -ItemType Directory -Force "$env:ProgramData\obs-studio\plugins\mobcam\data"
   ```
2. Copy `mobcam.dll` to `%ProgramData%\obs-studio\plugins\mobcam\bin\64bit\`.
3. Copy the contents of the `data\` folder to `%ProgramData%\obs-studio\plugins\mobcam\data\`.

#### Portable OBS Studio Location:
1. Navigate to your portable OBS Studio root folder (where `bin/` and `obs-plugins/` reside).
2. Copy `mobcam.dll` into `<OBS-Root>\obs-plugins\64bit\`.
3. Copy the `data\` directory contents into `<OBS-Root>\data\obs-plugins\mobcam\`.
4. Launch `obs64.exe`.

---

## 3. Connecting Your Android Device to OBS Studio

MobCam supports two peer-to-peer connection modes across your local environment:

### Mode 1: USB Connection (Ultra-Low Latency & High Stability)

1. Connect your Android phone to your PC using a reliable USB-C cable.
2. Enable **USB Tethering** in Android Settings (*Settings > Network & internet > Hotspot & tethering > USB tethering*).
3. Open the **MobCam** app on your phone and tap **Start Stream**.
4. In OBS Studio:
   - In the **Sources** dock, click **+** and select **MobCam Source**.
   - Keep **Select Camera Device** set to **[Auto-Detect (Seamless USB/Wi-Fi)]** or select your phone entry marked `[USB Wired: ...]`.
   - Click **OK** to begin broadcasting immediately.

---

### Mode 2: Wireless Wi-Fi Connection

1. Connect your Android phone and PC to the same local Wi-Fi network (5 GHz is recommended for 1080p and 4K streaming).
2. Open **MobCam** on your phone and tap **Start Stream**.
3. In OBS Studio:
   - Click **+** under **Sources** &rarr; **MobCam Source**.
   - The plugin's UDP beacon listener on port `4750` detects active phones on the subnet. Select your device from the dropdown (or select **Manual IP Entry** and enter the IP address displayed on your phone).
   - Click **OK** to stream.

---

## 4. In-OBS Studio Quick Controls Dock

MobCam embeds a dedicated control panel inside OBS Studio:

1. In OBS Studio, click on **Docks** in the top menu bar.
2. Select **MobCam Controls** (or add a Custom Browser Dock with URL `http://localhost:4752/`).
3. Dock this panel anywhere inside your OBS layout to manage:
   - Zoom presets (1x, 1.5x, 2x)
   - Flashlight / Torch toggle
   - Autofocus modes
   - Exposure compensation sliders

---

## 5. Network Ports Summary

| Port | Protocol | Purpose | Direction |
|---|---|---|---|
| **4750** | UDP | Device auto-discovery beacon | Inbound on PC (from Android) |
| **4747** | TCP | Control handshake & commands | Outbound from PC (to Android) |
| **4748** | TCP | H.264 / HEVC video stream | Outbound from PC (to Android) |
| **4749** | TCP | AAC audio stream | Outbound from PC (to Android) |
| **4752** | HTTP | Local Quick Controls dock UI | Local loopback (`127.0.0.1`) only |

---

## 6. Uninstalling the Plugin

- Go to **Windows Settings > Apps > Installed apps**.
- Find **MobCam OBS Studio Plugin** and click **Uninstall**.
- Alternatively, run the uninstaller directly at `%ProgramData%\obs-studio\plugins\mobcam\unins000.exe`.
- *Note: Uninstalling the plugin will not delete your personal OBS scene collections or configuration profiles.*

---

## 7. Trademarks & Legal Notice

OBS® and OBS Studio® are registered trademarks of Wizards of OBS LLC. MobCam is an independent open-source project and is not affiliated with, sponsored by, or endorsed by the OBS Project or Wizards of OBS LLC.
