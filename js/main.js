/**
 * MobCam - OBS Camera | Official Production Web Engine
 * Developed by Yash Rayjada | info@ybrdigital.in
 * Repository: https://github.com/ybrdigital/mobcam
 */

// ============================================================================
// 1. OBS Plugin Releases Catalog (Version Management Architecture)
// ============================================================================
const PLUGIN_RELEASES = [
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
    apkPath: "assets/downloads/v1.0.0/MobCam-Android-v1.0.0.apk",
    apkSize: "4.5 MB",
    apkSha256: "411478394952495AC26B9693923008CFF5E03E6EEAD55EB235B00977776B9F27",
    changelog: [
      { type: "badge-feat", label: "NEW", text: "Production-ready Windows 64-bit installer for OBS Studio 28.0 - 31.0+." },
      { type: "badge-feat", label: "NEW", text: "Ultra-low latency streaming engine (<30ms wired USB, <50ms 5GHz Wi-Fi)." },
      { type: "badge-feat", label: "NEW", text: "Hardware-accelerated YUV420 color rendering pipeline with zero chroma ghosting." },
      { type: "badge-feat", label: "NEW", text: "Integrated OBS Quick Controls web dock hosted natively on http://localhost:4752/." },
      { type: "badge-feat", label: "NEW", text: "Auto 16:9 landscape orientation lock prevents vertical pillarboxing." },
      { type: "badge-opt",  label: "OPTIMIZATION", text: "Dynamic network reconnect: automatic IP recovery upon router handoffs." },
      { type: "badge-fix",  label: "FIX", text: "Centered clean white watermark in OBS for free users with pure transparency." },
      { type: "badge-sec",  label: "SECURITY", text: "Isolated peer-to-peer TCP transmission without third-party cloud servers." }
    ]
  }
];

// ============================================================================
// 2. DOM Ready Initialization
// ============================================================================
$(document).ready(function () {
  renderReleasesArchive();
  initCopyChecksum();
  initSmoothScroll();
  initDownloadFeedback();
  initNavbarScroll();
  setDynamicYear();
});

// ============================================================================
// 3. Render Plugin Releases & Changelog
// ============================================================================
function renderReleasesArchive() {
  const $container = $("#releasesArchiveList");
  if (!$container.length) return;

  $container.empty();

  PLUGIN_RELEASES.forEach((rel, index) => {
    const isLatest = rel.isLatest;
    const latestBadge = isLatest
      ? `<span class="badge-latest ms-2"><i class="fas fa-sparkles me-1"></i>LATEST STABLE</span>`
      : `<span class="badge bg-secondary ms-2">ARCHIVED</span>`;

    let changelogHtml = "";
    rel.changelog.forEach(item => {
      let badgeClass = "badge-feat";
      if (item.label === "FIX") badgeClass = "badge-fix";
      if (item.label === "OPTIMIZATION") badgeClass = "badge-opt";
      if (item.label === "SECURITY") badgeClass = "badge-sec";

      changelogHtml += `
        <li>
          <span class="change-tag ${badgeClass}">${item.label}</span>
          <span>${item.text}</span>
        </li>
      `;
    });

    const releaseCardHtml = `
      <div class="release-card ${isLatest ? 'latest' : ''}" id="release-${rel.version}">
        <div class="d-flex flex-wrap align-items-center justify-content-between gap-3 mb-3">
          <div class="d-flex align-items-center flex-wrap gap-2">
            <span class="badge-version">v${rel.version}</span>
            ${latestBadge}
            <span class="text-muted-custom fs-sm"><i class="far fa-calendar-alt me-1"></i>Released: ${rel.releaseDate}</span>
            <span class="badge bg-dark border border-secondary text-dim fs-xs"><i class="fab fa-windows me-1"></i>${rel.windowsCompatibility}</span>
            <span class="badge bg-dark border border-secondary text-dim fs-xs"><i class="fas fa-video me-1"></i>${rel.obsCompatibility}</span>
          </div>
          <div class="d-flex align-items-center gap-2">
            <a href="${rel.installerPath}" class="btn btn-sm btn-primary-green download-btn" download data-version="${rel.version}">
              <i class="fas fa-download me-1"></i> Download Setup.exe (${rel.installerSize})
            </a>
            <a href="${rel.apkPath}" class="btn btn-sm btn-outline-secondary text-light" download title="Download Android APK">
              <i class="fab fa-android text-green me-1"></i> APK (${rel.apkSize})
            </a>
          </div>
        </div>

        <div class="changelog-box mb-3">
          <h6 class="text-white fw-bold mb-2 fs-sm text-uppercase tracking-wider">
            <i class="fas fa-list-check text-green me-2"></i>Release Notes & Improvements
          </h6>
          <ul class="release-notes-list mb-0">
            ${changelogHtml}
          </ul>
        </div>

        <div class="checksum-box">
          <div class="d-flex align-items-center gap-2 flex-grow-1 overflow-hidden">
            <span class="text-green fw-bold fs-xs"><i class="fas fa-shield-halved me-1"></i>SHA-256:</span>
            <code class="text-muted fs-xs text-truncate">${rel.sha256}</code>
          </div>
          <button class="btn-copy-checksum" data-hash="${rel.sha256}" title="Copy SHA-256 Checksum">
            <i class="far fa-copy me-1"></i><span>Copy</span>
          </button>
        </div>
      </div>
    `;

    $container.append(releaseCardHtml);
  });
}

// ============================================================================
// 4. Checksum Clipboard Copy Handler
// ============================================================================
function initCopyChecksum() {
  $(document).on("click", ".btn-copy-checksum", function (e) {
    e.preventDefault();
    const hash = $(this).attr("data-hash");
    const $btn = $(this);

    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(hash).then(() => {
        showCopyFeedback($btn);
      }).catch(() => {
        fallbackCopyText(hash, $btn);
      });
    } else {
      fallbackCopyText(hash, $btn);
    }
  });
}

function showCopyFeedback($btn) {
  const originalHtml = $btn.html();
  $btn.html('<i class="fas fa-check text-green me-1"></i><span class="text-green">Copied!</span>');
  setTimeout(() => {
    $btn.html(originalHtml);
  }, 2200);
}

function fallbackCopyText(text, $btn) {
  const $temp = $("<input>");
  $("body").append($temp);
  $temp.val(text).select();
  document.execCommand("copy");
  $temp.remove();
  showCopyFeedback($btn);
}

// ============================================================================
// 5. Smooth Scrolling for Navigation
// ============================================================================
function initSmoothScroll() {
  $('a.nav-link[href^="#"], a.smooth-scroll[href^="#"]').on("click", function (e) {
    const target = $(this.getAttribute("href"));
    if (target.length) {
      e.preventDefault();
      // Close mobile navbar if open
      const $navbarCollapse = $(".navbar-collapse");
      if ($navbarCollapse.hasClass("show")) {
        $navbarCollapse.collapse("hide");
      }

      $("html, body").stop().animate({
        scrollTop: target.offset().top - 80
      }, 500);
    }
  });
}

// ============================================================================
// 6. Download Tracking & In-Page Instructions Toast
// ============================================================================
function initDownloadFeedback() {
  $(document).on("click", ".download-btn", function () {
    const ver = $(this).data("version") || "1.0.0";
    showDownloadToast(ver);
  });
}

function showDownloadToast(version) {
  const toastHtml = `
    <div class="download-toast shadow-lg" id="dlToast">
      <div class="d-flex align-items-center gap-3">
        <div class="toast-icon">
          <i class="fas fa-check-circle text-green fs-4"></i>
        </div>
        <div class="toast-body-content">
          <h6 class="mb-1 text-white fw-bold">Downloading MobCam Plugin v${version}</h6>
          <p class="mb-0 text-muted fs-xs">
            Run the setup installer, restart OBS Studio, and add a new <strong>"MobCam Source"</strong> in your scene.
          </p>
        </div>
        <button type="button" class="btn-close btn-close-white ms-auto" aria-label="Close" onclick="$('#dlToast').fadeOut(300, function(){ $(this).remove(); });"></button>
      </div>
    </div>
  `;

  // Remove existing toast if any
  $("#dlToast").remove();
  $("body").append(toastHtml);
  $("#dlToast").fadeIn(300);

  // Auto-dismiss after 7 seconds
  setTimeout(() => {
    $("#dlToast").fadeOut(400, function () {
      $(this).remove();
    });
  }, 7000);
}

// ============================================================================
// 7. Navbar Styling on Scroll
// ============================================================================
function initNavbarScroll() {
  const $nav = $(".navbar-custom");
  $(window).on("scroll", function () {
    if ($(window).scrollTop() > 40) {
      $nav.addClass("scrolled");
    } else {
      $nav.removeClass("scrolled");
    }
  });
}

// ============================================================================
// 8. Dynamic Copyright Year
// ============================================================================
function setDynamicYear() {
  const currentYear = new Date().getFullYear();
  $("#currentYear").text(currentYear);
}
