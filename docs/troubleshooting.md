# MobCam OBS Studio Plugin - Troubleshooting Guide

This guide covers common issues, diagnostics, and solutions when using MobCam with OBS Studio.

---

## 1. MobCam Source is Missing in OBS Studio

### Symptoms
When clicking **+** under OBS Sources, **MobCam Source** does not appear.

### Solutions
1. **Check Plugin Location**:
   Verify that `mobcam.dll` is located at:
   ```text
   C:\ProgramData\obs-studio\plugins\mobcam\bin\64bit\mobcam.dll
   ```
2. **OBS Studio Architecture**:
   Ensure you are running the 64-bit version of OBS Studio 28.0 or newer. 32-bit OBS is not supported.
3. **Missing Visual C++ Redistributables**:
   Ensure you have the latest Microsoft Visual C++ 2015–2022 Redistributable (x64) installed.
4. **Check OBS Log**:
   In OBS, go to **Help > Log Files > View Current Log** and search for `[MobCam]`. Look for any error code during module loading.

---

## 2. Cannot Connect: "Connection Refused" or "Socket Timeout"

### Symptoms
OBS displays a connecting message or black screen with timeout errors.

### Solutions
1. **Verify Phone Stream is Active**:
   Ensure the MobCam app on Android is open and that the streaming service is actively running (you should see the camera preview and live port `4747`).
2. **Same Subnet / AP Isolation**:
   - Both devices must be on the same local subnet (e.g., PC on `192.168.1.10`, Phone on `192.168.1.45`).
   - Many guest Wi-Fi networks and mesh routers enable **AP Isolation** (Client Isolation), which blocks device-to-device communication. Disable AP Isolation in your router settings or connect phone and PC to the primary Wi-Fi network.
3. **Windows Defender Firewall**:
   Windows Firewall may block incoming or outgoing socket connections. Add an inbound rule in Windows Defender Firewall for TCP port `4747` and UDP port `4748`.
4. **PIN Mismatch**:
   Verify the 6-digit PIN entered in OBS source properties matches the PIN displayed on the phone.

---

## 3. High Latency or Video Stuttering

### Symptoms
Noticeable video delay (> 100ms) or dropped frames.

### Solutions
1. **Switch to 5 GHz Wi-Fi**:
   2.4 GHz Wi-Fi experiences heavy channel congestion and Bluetooth interference. Switching to a 5 GHz band significantly reduces latency and packet jitter.
2. **Use Wired USB Tethering**:
   Wired USB connection provides the lowest possible latency (< 30ms) and avoids wireless interference entirely.
3. **Lower Stream Resolution or Bitrate**:
   If your phone or PC has limited hardware encoding/decoding capacity:
   - In MobCam Android settings, select **1080p @ 60 FPS** or **720p @ 60 FPS**.
   - Lower bitrate to 8,000–12,000 kbps for 1080p.
4. **Thermal Throttling**:
   Prolonged 4K streaming causes phone processors to heat up and throttle. Keep the device ventilated or use 1080p for extended live streams.

---

## 4. Audio Desynchronization

### Symptoms
Audio leads or lags behind the camera video stream.

### Solutions
1. In OBS Studio, open **Edit > Advanced Audio Properties**.
2. Locate the **MobCam** audio track.
3. Adjust the **Sync Offset (ms)** (typically -50ms to +50ms depending on audio interface).
4. Verify that OBS Studio audio sample rate (**Settings > Audio > Sample Rate**) is set to **48 kHz** to match MobCam's native stream.

---

## 5. In-OBS Quick Controls Dock Not Loading

### Symptoms
The dock window displays "Unable to connect" or a blank page.

### Solutions
1. Verify the local web server port: `http://localhost:4752/` in any desktop browser.
2. Ensure no third-party background software or local development server is occupying port `4752`.
3. Check OBS logs for `[MobCam] Plugin loaded successfully with Auto-Discovery & Quick Controls Web Server`.
4. In OBS, go to **Docks > Custom Browser Docks** and refresh or re-add the dock with URL `http://localhost:4752/`.

---

## 6. Still Having Issues?

- Visit our [GitHub Issues](https://github.com/ybrdigital/mobcam/issues) to view reported bugs or open a new issue.
- Contact our technical support at **[info@ybrdigital.in](mailto:info@ybrdigital.in)** with your OBS log file and device specifications.
