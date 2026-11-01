# MobCam - Native Camera Plugin for OBS Studio
## Troubleshooting & Diagnostics Guide

This guide covers common issues, diagnostic procedures, firewall configuration, and performance optimization when using MobCam with OBS Studio.

---

## 1. MobCam Source is Missing in OBS Studio

### Symptoms
When clicking **+** under OBS Sources, **MobCam Source** does not appear.

### Solutions
1. **Check Plugin Binary Placement**:
   Verify that `mobcam.dll` is located at:
   ```text
   C:\ProgramData\obs-studio\plugins\mobcam\bin\64bit\mobcam.dll
   ```
   Or for portable OBS installations:
   ```text
   <OBS-Root>\obs-plugins\64bit\mobcam.dll
   ```
2. **Architecture Compatibility**:
   Ensure you are running the native 64-bit version of OBS Studio 28.0 or newer. 32-bit OBS is not supported.
3. **Visual C++ Redistributable**:
   Ensure the latest Microsoft Visual C++ 2015–2022 Redistributable (x64) is installed.
4. **Check OBS Log Files**:
   In OBS, go to **Help > Log Files > View Current Log** and search for `[MobCam]`. Look for any error code during module loading.

---

## 2. Cannot Connect: "Connection Refused" or "Socket Timeout"

### Symptoms
OBS displays a connecting message or black screen with timeout errors.

### Solutions
1. **Verify Phone Stream is Active**:
   Ensure the MobCam app on Android is open and that the streaming service is running (you should see the live camera viewfinder).
2. **Same Subnet / AP Isolation**:
   - Both devices must be on the same local network subnet (e.g., PC on `192.168.1.10`, Phone on `192.168.1.45`).
   - Many guest Wi-Fi networks and public/mesh routers enable **AP Isolation** (Client Isolation), which blocks device-to-device communication. Disable AP Isolation in your router settings or connect both devices to your primary Wi-Fi network.
3. **Firewall & Network Port Permissions**:
   Windows Defender Firewall may block incoming discovery beacons or outgoing TCP sockets. Add firewall rules using PowerShell (Run as Administrator):
   ```powershell
   # Allow incoming UDP discovery beacons on port 4750
   New-NetFirewallRule -DisplayName "MobCam OBS Plugin - UDP Discovery" -Direction Inbound -LocalPort 4750 -Protocol UDP -Action Allow

   # Allow outbound TCP media streaming & controls
   New-NetFirewallRule -DisplayName "MobCam OBS Plugin - TCP Media & Control" -Direction Outbound -RemotePort 4747,4748,4749 -Protocol TCP -Action Allow
   ```
4. **Manual IP Fallback**:
   If auto-discovery is delayed by local network broadcast policies, choose **[Manual IP Entry]** in the OBS MobCam Source properties and enter the IP address displayed on your phone's screen.

---

## 3. Network Ports Reference Table

| Purpose | Port & Protocol | Direction | Listening Entity |
|---|---|---|---|
| **Device Discovery** | UDP 4750 | Inbound on PC | OBS Plugin (`discovery_listener.c`) |
| **Control Connection** | TCP 4747 | Outbound to Phone | Android App |
| **Video Media Stream** | TCP 4748 | Outbound to Phone | Android App |
| **Audio Media Stream** | TCP 4749 | Outbound to Phone | Android App |
| **Quick Controls Dock** | HTTP TCP 4752 | Loopback (`127.0.0.1`) | OBS Plugin (`web_control_server.c`) |

---

## 4. High Latency or Video Stuttering

### Symptoms
Noticeable video delay (> 100ms) or dropped video frames.

### Solutions
1. **Switch to 5 GHz Wi-Fi**:
   2.4 GHz Wi-Fi experiences heavy channel congestion and interference. Switching to a 5 GHz band significantly reduces latency and packet jitter.
2. **Use Wired USB Tethering**:
   Wired USB connection provides the lowest possible latency (< 30ms) and avoids radio interference entirely.
3. **Lower Stream Resolution or Bitrate**:
   If your phone or PC has limited hardware encoding/decoding capacity:
   - In MobCam Android settings, select **1080p @ 60 FPS** or **720p @ 60 FPS**.
   - Lower bitrate to 8,000–12,000 kbps for 1080p.
4. **Device Thermal Throttling**:
   Prolonged 4K streaming causes phone processors to heat up and throttle. Keep the device ventilated or use 1080p for extended live streams.

---

## 5. Audio Desynchronization

### Symptoms
Audio leads or lags behind the camera video stream.

### Solutions
1. In OBS Studio, open **Edit > Advanced Audio Properties**.
2. Locate the **MobCam** audio track.
3. Adjust the **Sync Offset (ms)** (typically -50ms to +50ms depending on system audio interface).
4. Verify that OBS Studio audio sample rate (**Settings > Audio > Sample Rate**) is set to **48 kHz** to match MobCam's native stream.

---

## 6. In-OBS Quick Controls Dock Not Loading

### Symptoms
The dock window displays "Unable to connect" or a blank page.

### Solutions
1. Verify the local web server port: `http://localhost:4752/` in any desktop browser.
2. Ensure no third-party background software or local development server is occupying port `4752`.
3. Check OBS logs for `[MobCam] Web control server running at http://localhost:4752/`.
4. In OBS, go to **Docks > Custom Browser Docks** and refresh or re-add the dock with URL `http://localhost:4752/`.

---

## 7. Trademarks & Legal Notice

OBS® and OBS Studio® are registered trademarks of Wizards of OBS LLC. MobCam is an independent open-source project and is not affiliated with, sponsored by, or endorsed by the OBS Project or Wizards of OBS LLC.
