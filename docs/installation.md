# MobCam OBS Studio Plugin - Installation & Setup Guide

This guide walks you through installing and configuring the **MobCam OBS Studio Plugin** on Windows 10/11 and connecting it to your Android device running MobCam.

---

## 1. System Requirements

### Windows PC
- **OS**: Windows 10 or Windows 11 (64-bit)
- **OBS Studio**: OBS Studio 28.0 - 31.0+ (64-bit native)
- **Network**: Wired Ethernet / 5 GHz Wi-Fi or USB connection

### Android Device
- **OS**: Android 8.0 Oreo (API Level 26) or newer (Android 10+ recommended)
- **Hardware**: Camera with Camera2 API / HAL3 support
- **App**: [MobCam on Google Play Store](https://play.google.com/store/apps/details?id=com.ybrdigital.mobcam)

---

## 2. Installation Options

### Option A: Windows Installer (Recommended)

1. **Download the Installer**:
   Download `MobCam-OBS-Plugin-v1.0.0-Setup.exe` from the [Releases page](https://github.com/ybrdigital/mobcam/releases) or the [official landing page](https://ybrdigital.github.io/mobcam/).

2. **Close OBS Studio**:
   Ensure OBS Studio (`obs64.exe`) is completely closed before running the setup wizard.

3. **Run the Installer**:
   Double-click `MobCam-OBS-Plugin-v1.0.0-Setup.exe` and follow the prompts. The installer will:
   - Verify your OBS Studio installation.
   - Install `mobcam.dll` into `%ProgramData%\obs-studio\plugins\mobcam\bin\64bit\`.
   - Install locale and icon resources into `%ProgramData%\obs-studio\plugins\mobcam\data\`.
   - Register the uninstaller in Windows Add/Remove Programs.

4. **Launch OBS Studio**:
   Open OBS Studio. You will now see **MobCam Source** available in the Sources menu.

---

### Option B: Manual Installation from Source / Binary

If you built `mobcam.dll` from source:

1. Create the plugin directories if they do not exist:
   ```powershell
   New-Item -ItemType Directory -Force "$env:ProgramData\obs-studio\plugins\mobcam\bin\64bit"
   New-Item -ItemType Directory -Force "$env:ProgramData\obs-studio\plugins\mobcam\data\locale"
   ```

2. Copy the binary:
   ```powershell
   Copy-Item "plugin\build\bin\Release\mobcam.dll" "$env:ProgramData\obs-studio\plugins\mobcam\bin\64bit\"
   ```

3. Copy data and locale files:
   ```powershell
   Copy-Item -Recurse "plugin\data\*" "$env:ProgramData\obs-studio\plugins\mobcam\data\"
   ```

4. Restart OBS Studio.

---

## 3. Connecting Your Android Device to OBS Studio

### Mode 1: USB Connection (Lowest Latency & Maximum Stability)

1. Connect your Android phone to your PC using a high-quality USB-C cable.
2. Enable **USB Tethering** in Android Settings (*Settings > Network & internet > Hotspot & tethering > USB tethering*).
3. Open the **MobCam** app on your phone and tap **Start Stream**.
4. In OBS Studio:
   - In the **Sources** dock, click **+** and select **MobCam Source**.
   - Keep **Select Camera Device** set to **[Auto-Detect (Seamless USB/Wi-Fi)]** or select your phone marked with `[USB Wired: ...]`.
   - Click **OK** to immediately start streaming.

---

### Mode 2: Wireless Wi-Fi Connection

1. Connect your Android phone and PC to the same Wi-Fi network (5 GHz Wi-Fi is strongly recommended for 1080p and 4K streaming).
2. Open **MobCam** on your phone and tap **Start Stream**.
3. In OBS Studio:
   - Click **+** under **Sources** &rarr; **MobCam Source**.
   - With auto-discovery active, your phone will appear in the dropdown list automatically (or choose **Manual IP Entry** and enter the IP shown in the app).
   - Click **OK** to begin streaming immediately.

---

## 4. Enabling In-OBS Studio Quick Controls Dock

MobCam includes a dedicated dock inside OBS Studio:

1. In OBS Studio, click on **Docks** in the top menu bar.
2. Select **MobCam Controls** (or add a Custom Browser Dock with URL `http://localhost:4752/`).
3. You can dock this panel anywhere inside your OBS layout to adjust:
   - Zoom presets (1x, 1.5x, 2x)
   - Flashlight / Torch toggle
   - Autofocus modes (Continuous, Touch, Macro)
   - Exposure compensation sliders

---

## 5. Uninstalling the Plugin

- Go to **Windows Settings > Apps > Installed apps**.
- Find **MobCam OBS Studio Plugin** and click **Uninstall**.
- Or run the uninstaller located at `%ProgramData%\obs-studio\plugins\mobcam\unins000.exe`.
