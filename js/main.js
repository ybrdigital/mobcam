/**
 * MobCam - OBS Studio Camera | Modern Web Engine
 * Developer: Yash Rayjada (YBR Digital)
 * Repository: https://github.com/ybrdigital/mobcam
 * License: GPL-2.0-or-later
 */

// ============================================================================
// 1. Central Version Configuration & Fallback Definition
// ============================================================================
const DEFAULT_VERSION_CONFIG = {
  product: "MobCam OBS Studio Plugin",
  version: "1.0.0",
  tag: "v1.0.0",
  releaseDate: "September 25, 2026",
  windowsCompatibility: "Windows 10 / 11 (64-bit)",
  obsCompatibility: "OBS Studio 28.0 - 31.0+ (64-bit)",
  architecture: "x64",
  installerFilename: "MobCam-OBS-Plugin-v1.0.0-Setup.exe",
  installerPath: "assets/downloads/v1.0.0/MobCam-OBS-Plugin-v1.0.0-Setup.exe",
  downloadUrl: "assets/downloads/v1.0.0/MobCam-OBS-Plugin-v1.0.0-Setup.exe",
  githubReleaseUrl: "https://github.com/ybrdigital/mobcam/releases",
  fileSize: "2.1 MB",
  sha256: "A33CD424E25D638B5BA3920264BE76C270F2B61FD7E0014817AEA5D5F84BFEEB"
};

let activeVersionConfig = { ...DEFAULT_VERSION_CONFIG };

// ============================================================================
// 2. DOM Ready Initialization
// ============================================================================
document.addEventListener("DOMContentLoaded", () => {
  initVersionData();
  initCopyChecksum();
  initSmoothScroll();
  initNavbarScroll();
  initScreenshotLightbox();
  setDynamicYear();
});

// ============================================================================
// 3. Central Version Data Loader (Local JSON + GitHub API Fallback)
// ============================================================================
async function initVersionData() {
  try {
    const res = await fetch("assets/version.json", { cache: "no-store" });
    if (res.ok) {
      const data = await res.json();
      activeVersionConfig = { ...DEFAULT_VERSION_CONFIG, ...data };
    }
  } catch (err) {
    console.warn("[MobCam] Using local fallback version config:", err);
  }

  // Optionally query GitHub API for releases without exposing credentials
  try {
    const ghRes = await fetch("https://api.github.com/repos/ybrdigital/mobcam/releases/latest", {
      headers: { Accept: "application/vnd.github.v3+json" }
    });
    if (ghRes.ok) {
      const ghData = await ghRes.json();
      if (ghData && ghData.assets && ghData.assets.length > 0) {
        const setupAsset = ghData.assets.find(a => a.name.endsWith(".exe") || a.name.includes("Setup"));
        if (setupAsset) {
          activeVersionConfig.downloadUrl = setupAsset.browser_download_url;
          activeVersionConfig.installerFilename = setupAsset.name;
          activeVersionConfig.fileSize = `${(setupAsset.size / (1024 * 1024)).toFixed(1)} MB`;
        }
      }
      if (ghData.tag_name) {
        activeVersionConfig.version = ghData.tag_name.replace(/^v/, "");
        activeVersionConfig.tag = ghData.tag_name;
      }
    }
  } catch (ghErr) {
    // Graceful fallback to verified bundled asset
    console.debug("[MobCam] GitHub Releases API query skipped or rate-limited:", ghErr);
  }

  applyVersionDataToUI(activeVersionConfig);
}

function applyVersionDataToUI(cfg) {
  // Update all download buttons
  const downloadBtns = document.querySelectorAll(".primary-download-btn");
  downloadBtns.forEach(btn => {
    btn.setAttribute("href", cfg.downloadUrl);
    btn.setAttribute("download", cfg.installerFilename);
  });

  // Update dynamic text elements
  const elVer = document.querySelectorAll(".dynamic-version-tag");
  elVer.forEach(el => el.textContent = `v${cfg.version}`);

  const elSize = document.querySelectorAll(".dynamic-file-size");
  elSize.forEach(el => el.textContent = cfg.fileSize);

  const elDate = document.querySelectorAll(".dynamic-release-date");
  elDate.forEach(el => el.textContent = cfg.releaseDate);

  const elHash = document.querySelectorAll(".dynamic-sha256");
  elHash.forEach(el => el.textContent = cfg.sha256);

  const elCopy = document.querySelectorAll(".btn-copy-checksum");
  elCopy.forEach(btn => btn.setAttribute("data-hash", cfg.sha256));
}

// ============================================================================
// 4. SHA-256 Checksum Copy Helper
// ============================================================================
function initCopyChecksum() {
  document.addEventListener("click", (e) => {
    const btn = e.target.closest(".btn-copy-checksum");
    if (!btn) return;

    const hash = btn.getAttribute("data-hash") || activeVersionConfig.sha256;
    if (!hash) return;

    navigator.clipboard.writeText(hash).then(() => {
      const origText = btn.innerHTML;
      btn.innerHTML = `<i class="fas fa-check text-green me-1"></i><span>Copied!</span>`;
      btn.classList.add("copied");
      setTimeout(() => {
        btn.innerHTML = origText;
        btn.classList.remove("copied");
      }, 2500);
    }).catch(err => {
      console.error("Failed to copy checksum:", err);
    });
  });
}

// ============================================================================
// 5. Smooth Scroll Navigation
// ============================================================================
function initSmoothScroll() {
  document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener("click", function(e) {
      const targetId = this.getAttribute("href");
      if (targetId === "#" || targetId.length <= 1) return;

      const targetEl = document.querySelector(targetId);
      if (targetEl) {
        e.preventDefault();
        const navHeight = document.querySelector("#mainNav")?.offsetHeight || 70;
        const targetPosition = targetEl.getBoundingClientRect().top + window.pageYOffset - navHeight - 16;
        window.scrollTo({
          top: targetPosition,
          behavior: "smooth"
        });

        // Close mobile navbar if open
        const navCollapse = document.querySelector(".navbar-collapse");
        if (navCollapse && navCollapse.classList.contains("show")) {
          const bsCollapse = bootstrap.Collapse.getInstance(navCollapse);
          if (bsCollapse) bsCollapse.hide();
        }
      }
    });
  });
}

// ============================================================================
// 6. Sticky Navbar Background Blur & Scroll State
// ============================================================================
function initNavbarScroll() {
  const navbar = document.querySelector("#mainNav");
  if (!navbar) return;

  const handleScroll = () => {
    if (window.scrollY > 40) {
      navbar.classList.add("scrolled");
    } else {
      navbar.classList.remove("scrolled");
    }
  };

  window.addEventListener("scroll", handleScroll, { passive: true });
  handleScroll();
}

// ============================================================================
// 7. Screenshot Lightbox / Modal Viewer
// ============================================================================
function initScreenshotLightbox() {
  const modalEl = document.getElementById("screenshotModal");
  if (!modalEl) return;

  const modalImg = modalEl.querySelector(".modal-screenshot-img");
  const modalTitle = modalEl.querySelector(".modal-title");

  document.querySelectorAll("[data-bs-toggle='lightbox']").forEach(item => {
    item.addEventListener("click", (e) => {
      e.preventDefault();
      const imgSrc = item.getAttribute("data-src") || item.getAttribute("href");
      const imgAlt = item.getAttribute("data-caption") || item.querySelector("img")?.alt || "Screenshot";
      if (modalImg && imgSrc) {
        modalImg.src = imgSrc;
        modalImg.alt = imgAlt;
      }
      if (modalTitle) {
        modalTitle.textContent = imgAlt;
      }
      const bsModal = new bootstrap.Modal(modalEl);
      bsModal.show();
    });
  });
}

// ============================================================================
// 8. Dynamic Copyright Year
// ============================================================================
function setDynamicYear() {
  const yearEl = document.getElementById("currentYear");
  if (yearEl) {
    yearEl.textContent = new Date().getFullYear();
  }
}
