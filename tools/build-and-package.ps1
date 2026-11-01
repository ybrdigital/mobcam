# MobCam - Manual Build and Packaging Tool for Maintainer
# Author: Yash Rayjada (YBR Digital)
# Usage: Run from powershell: .\tools\build-and-package.ps1

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path
$pluginDir = Join-Path $repoRoot "plugin"
$buildDir = Join-Path $pluginDir "build"
$releaseDir = Join-Path $repoRoot "release"
$issPath = Join-Path $pluginDir "installer\setup.iss"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " MobCam OBS Studio Plugin - Manual Build & Package Tool" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# 1. Locate CMake
$cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
$cmake = if ($cmakeCmd) { $cmakeCmd.Source } else { $null }
if (-not $cmake) {
    $vsCmake = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    $vs2022Cmake = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path $vsCmake) { $cmake = $vsCmake }
    elseif (Test-Path $vs2022Cmake) { $cmake = $vs2022Cmake }
    elseif (Test-Path "C:\Program Files\CMake\bin\cmake.exe") { $cmake = "C:\Program Files\CMake\bin\cmake.exe" }
    else { throw "Error: cmake.exe was not found. Please install CMake or Visual Studio C++ workload." }
}
Write-Host "[1/4] Using CMake: $cmake" -ForegroundColor Green

# 2. Configure & Build Plugin (Release x64)
Write-Host "[2/4] Building Plugin (MSVC Release x64)..." -ForegroundColor Yellow
Push-Location $pluginDir
try {
    & $cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
    & $cmake --build build --config Release
} finally {
    Pop-Location
}

$dllPath = Join-Path $buildDir "bin\Release\mobcam.dll"
if (-not (Test-Path $dllPath)) {
    throw "Error: mobcam.dll was not generated at $dllPath"
}
$dllHash = (Get-FileHash $dllPath -Algorithm SHA256).Hash
Write-Host "      Generated DLL: $dllPath" -ForegroundColor Green
Write-Host "      DLL SHA-256:   $dllHash" -ForegroundColor DarkGray

# 3. Locate Inno Setup & Compile Installer
Write-Host "[3/4] Compiling Windows Setup Installer with Inno Setup..." -ForegroundColor Yellow
$isccCandidates = @(
    "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe",
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    "C:\Program Files\Inno Setup 6\ISCC.exe"
)
$iscc = $isccCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $iscc) {
    $isccCmd = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($isccCmd) { $iscc = $isccCmd.Source }
}
if (-not $iscc) {
    throw "Error: Inno Setup 6 compiler (ISCC.exe) was not found. Please install Inno Setup 6."
}
Write-Host "      Using ISCC: $iscc" -ForegroundColor Green

if (-not (Test-Path $releaseDir)) {
    New-Item -ItemType Directory -Force $releaseDir | Out-Null
}

& $iscc $issPath

$setupExe = Join-Path $releaseDir "MobCam-OBS-Plugin-v1.0.0-Setup.exe"
if (-not (Test-Path $setupExe)) {
    throw "Error: Setup executable was not generated at $setupExe"
}

# 4. Generate SHA-256 Checksum
Write-Host "[4/4] Computing SHA-256 Checksums..." -ForegroundColor Yellow
$setupHash = (Get-FileHash $setupExe -Algorithm SHA256).Hash
$checksumFile = Join-Path $releaseDir "SHA256SUMS.txt"
"$setupHash  MobCam-OBS-Plugin-v1.0.0-Setup.exe" | Out-File -Encoding utf8 $checksumFile

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Build & Packaging Complete!" -ForegroundColor Green
Write-Host " Installer: $setupExe" -ForegroundColor White
Write-Host " Checksum:  $setupHash" -ForegroundColor White
Write-Host " File:      $checksumFile" -ForegroundColor White
Write-Host " Ready for manual upload to GitHub Releases: v1.0.0" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan
