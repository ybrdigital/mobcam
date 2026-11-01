# Third-Party Notices & Licenses

This document lists the third-party software, libraries, and components utilized by the **MobCam OBS Studio Plugin** and the official documentation website, along with their respective copyright notices and license terms.

---

## 1. OBS Studio Plugin Dependencies

### 1.1 libobs (OBS Studio Core API)
- **Project**: [OBS Studio](https://obsproject.com/)
- **Copyright**: &copy; 2012–2026 OBS Project and contributors
- **License**: GNU General Public License v2.0 or later (`GPL-2.0-or-later`)
- **Usage**: MobCam integrates with OBS Studio via `libobs` module interfaces (`obs-module.h`, `obs.h`, `obs_register_source()`).
- **Compatibility**: MobCam is licensed under `GPL-2.0-or-later`, ensuring full binary and source compatibility with OBS Studio's GPLv2+ ecosystem.

### 1.2 FFmpeg (libavcodec & libavutil)
- **Project**: [FFmpeg](https://ffmpeg.org/)
- **Copyright**: &copy; 2000–2026 the FFmpeg developers
- **License**: GNU Lesser General Public License v2.1 or later (`LGPL-2.1-or-later`) / GNU General Public License v2.0 or later (`GPL-2.0-or-later` when built with GPL components)
- **Usage**: Dynamically linked for H.264 video decoding and AAC audio decoding. At runtime in OBS Studio on Windows, MobCam dynamically links against `avcodec-62.dll` and `avutil-60.dll` already bundled and loaded in memory by OBS Studio.
- **Compliance Audit**: MobCam includes only standard C header definitions and links dynamically at runtime. Under Section 2 and 3 of GPLv2 and LGPLv2.1, dynamic linking does not encumber or restrict redistribution.

### 1.3 cJSON
- **Project**: [cJSON](https://github.com/DaveGamble/cJSON)
- **Copyright**: &copy; 2009–2017 Dave Gamble and cJSON contributors
- **License**: MIT License
- **License Text**:
  ```text
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  ```

### 1.4 Windows Winsock2 & Platform SDK
- **Provider**: Microsoft Corporation
- **License**: Microsoft Windows SDK Terms
- **Usage**: Standard Windows TCP/UDP networking (`ws2_32.lib`, `<winsock2.h>`, `<ws2tcpip.h>`).
- **GPL Exception**: Governed by the standard Operating System Component Exception of GPLv2 (Section 3).

### 1.5 Inno Setup
- **Project**: [Inno Setup](https://jrsoftware.org/isinfo.php)
- **Copyright**: &copy; 1997–2026 Jordan Russell, Martijn Laan
- **License**: Modified BSD-style Inno Setup License
- **Usage**: Used to compile the Windows 64-bit installer executable (`MobCam-OBS-Plugin-Setup.exe`).

---

## 2. Web Documentation & UI Dependencies

### 2.1 Bootstrap 5
- **Project**: [Bootstrap](https://getbootstrap.com/)
- **Copyright**: &copy; 2011–2026 The Bootstrap Authors
- **License**: MIT License

### 2.2 Font Awesome Free
- **Project**: [Font Awesome Free](https://fontawesome.com/)
- **Copyright**: &copy; Fonticons, Inc.
- **License**: Icons: CC BY 4.0 | Fonts: SIL OFL 1.1 | Code: MIT License

### 2.3 Plus Jakarta Sans & JetBrains Mono Fonts
- **Plus Jakarta Sans**: Copyright &copy; 2020 Gumpita Rahayu, Tokotype. Licensed under the SIL Open Font License 1.1.
- **JetBrains Mono**: Copyright &copy; 2020 JetBrains s.r.o. Licensed under the SIL Open Font License 1.1.

---

## 3. Source Code Availability & GPL Compliance

Under the terms of the **GNU General Public License v2.0 or later (GPL-2.0-or-later)**:
- The complete, corresponding source code for all official distributed binaries of the MobCam Windows OBS Studio plugin is publicly available at:  
  **[https://github.com/ybrdigital/mobcam](https://github.com/ybrdigital/mobcam)**
- Users and developers have the right to inspect, modify, build, and redistribute the plugin source code and derivative works in accordance with the GPL-2.0-or-later.
- Third-party runtime dependencies (`avcodec-62.dll`, `avutil-60.dll`, `obs.dll`) are provided dynamically by the user's local installation of OBS Studio.

---

## 4. Proprietary Notice: MobCam Android Application & Brand Assets

- **Proprietary Android Application**: The **MobCam Android Mobile Application** is a proprietary commercial product of **YBR Digital** and is **not** licensed under the GPL. It is distributed through the Google Play Store. The plugin communicates with the Android app over an independently specified local network protocol; this communication across independent network sockets does not subject the proprietary mobile app to the GPL.
- **Brand & Trademarks**: The name "MobCam", logo artwork, icons, website design, and documentation styling remain the exclusive proprietary assets and trademarks of YBR Digital and are not covered by the GPL.

---

## 5. Trademark Notices

- **OBS Studio**: OBS® and OBS Studio® are registered trademarks of Wizards of OBS LLC. MobCam is an independent third-party open-source project and is not affiliated with, sponsored by, or endorsed by the OBS Project or Wizards of OBS LLC.
- **Android & Google Play**: Android and Google Play are trademarks of Google LLC.
