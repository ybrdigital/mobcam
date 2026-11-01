# Changelog

All notable changes to the **MobCam OBS Studio Plugin** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0] - 2026-09-25

### Added
- **Native OBS Source Registration**: Registered `mobcam_source` (`MobCam Source`) natively in OBS Studio using `libobs`.
- **Hardware-Accelerated H.264 Video Decoder**: Direct TCP streaming pipeline utilizing `libavcodec` (FFmpeg) with YUV420 planar frame rendering and zero chroma ghosting.
- **Synchronized AAC Audio Pipeline**: 48 kHz stereo audio stream decoded from incoming media packets directly into the OBS Audio Mixer with timestamp synchronization.
- **Automatic Device Discovery**: UDP broadcast listener on port 4748 automatically detects active MobCam Android devices on the local subnet.
- **In-OBS Studio Quick Controls Dock**: Embedded HTTP web dock (`http://localhost:4752/`) accessible via `Docks > MobCam Controls` to adjust zoom presets (1x, 1.5x, 2x), autofocus modes, flashlight, and exposure compensation directly from OBS Studio.
- **Auto 16:9 Landscape Lock**: Prevents vertical pillarboxing by locking video feeds to horizontal aspect ratio.
- **USB Tethering & Wi-Fi Support**: Seamless connectivity over 5 GHz Wi-Fi or wired Android USB tethering with zero manual setup.
- **Inno Setup 6 Installer**: Automated Windows 64-bit installer with OBS Studio installation detection and process termination checks.
- **Documentation & Website**: Production landing page on GitHub Pages with downloads, checksums, and technical specifications.
