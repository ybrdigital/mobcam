# Contributing to MobCam OBS Studio Plugin

Thank you for your interest in contributing to the **MobCam OBS Studio Plugin**! We welcome bug reports, feature requests, documentation improvements, and pull requests from developers of all backgrounds.

---

## 1. Code of Conduct

We are committed to providing a friendly, safe, and welcoming environment for all contributors. Please be respectful and considerate in discussions, issue comments, and code reviews.

---

## 2. Scope of this Repository

This repository hosts:
1. **The Native MobCam OBS Studio Plugin** (`plugin/`), written in C, built for OBS Studio 28.0 - 31.0+ on Windows (x64).
2. **Official Documentation & Website** (`docs/`, `index.html`), hosted on GitHub Pages.

> **Note**: The **MobCam Android Mobile Application** is a proprietary product of YBR Digital distributed via the Google Play Store and is maintained in a private repository (MobCam operates entirely peer-to-peer with no backend servers). Please do not submit pull requests containing Android Java/Kotlin/Flutter code.

---

## 3. Development Workflow

### 3.1 Prerequisites
- **Operating System**: Windows 10 / Windows 11 (64-bit)
- **C Compiler**: Microsoft Visual Studio 2022 (MSVC v143 or newer) or Build Tools
- **Build System**: CMake 3.22 or higher
- **OBS Studio**: OBS Studio 28.0+ 64-bit installed for testing
- *(Optional)* **Inno Setup 6.x**: To compile the installer executable

### 3.2 Building from Source

1. **Clone the repository**:
   ```powershell
   git clone https://github.com/ybrdigital/mobcam.git
   cd mobcam/plugin
   ```

2. **Configure CMake**:
   ```powershell
   cmake -B build -S .
   ```

3. **Compile Release Binary**:
   ```powershell
   cmake --build build --config Release
   ```
   The compiled plugin binary will be generated at `build/bin/Release/mobcam.dll`.

4. **Install to OBS Studio (Manual Test)**:
   Copy `mobcam.dll` to:
   ```text
   %ProgramData%\obs-studio\plugins\mobcam\bin\64bit\mobcam.dll
   ```
   And data files to:
   ```text
   %ProgramData%\obs-studio\plugins\mobcam\data\
   ```
   Then launch OBS Studio and verify `MobCam Source` appears under the Sources menu.

---

## 4. Coding Standards

- **Language**: Standard C17.
- **Style**: Follow standard libobs style guidelines (4 spaces indentation, no tabs, clear naming conventions).
- **Header Guards**: Use `#ifndef MOBCAM_MODULE_NAME_H` guards.
- **SPDX Headers**: Every new source file must include the GPL-2.0-or-later header and `SPDX-License-Identifier: GPL-2.0-or-later`.
- **Memory Safety**: Check all allocations, free all buffers upon destruction, and prevent memory leaks.
- **Thread Safety**: Protect shared socket data and frame queues with appropriate mutexes and locks.

---

## 5. Submitting Pull Requests

1. Fork the repository and create your feature branch from `main`:
   ```bash
   git checkout -b feature/my-new-feature
   ```
2. Keep your commits atomic, well-described, and self-contained.
3. Test your changes in OBS Studio before submitting.
4. Push to your fork and submit a Pull Request to `ybrdigital/mobcam:main`.
5. Clearly describe the motivation, changes made, and verification steps in your PR description.

---

## 6. Reporting Issues

- Search existing issues to ensure the problem hasn't already been reported.
- Provide your OBS Studio version, Windows OS build, device model, and connection mode (Wi-Fi or USB).
- Include relevant OBS log excerpts (`Help > Log Files > View Current Log`).
