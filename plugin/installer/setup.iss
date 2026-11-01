; Inno Setup Script for MobCam OBS Studio Plugin
; Compatible with Inno Setup 6.x

#define MyAppName "MobCam Camera Plugin for OBS Studio"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "YBR Digital"
#define MyAppURL "https://ybrdigital.github.io/mobcam/"
#define MyAppExeName "MobCam-OBS-Plugin-v1.0.0-Setup.exe"

[Setup]
AppId={{9B78B454-942C-4A2D-A2A6-F3C2C910B8D9}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppComments=Native Windows camera plugin connecting MobCam Android to OBS Studio over Wi-Fi and USB.
AppContact=info@ybrdigital.in
DefaultDirName={commonappdata}\obs-studio\plugins\mobcam
DisableDirPage=yes
DisableProgramGroupPage=yes
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputBaseFilename=MobCam-OBS-Plugin-v1.0.0-Setup
OutputDir=..\..\release
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
SetupIconFile=mobcam.ico
UninstallDisplayIcon={app}\mobcam.ico
CreateUninstallRegKey=yes
LicenseFile=..\..\LICENSE
VersionInfoVersion=1.0.0.0
VersionInfoCompany=YBR Digital
VersionInfoDescription=MobCam OBS Studio Plugin Setup
VersionInfoProductName=MobCam
VersionInfoCopyright=Copyright (C) 2026 YBR Digital

[Messages]
BeveledLabel=MobCam is an independent open-source plugin compatible with OBS Studio.

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\THIRD_PARTY_LICENSES.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "mobcam.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build\bin\Release\mobcam.dll"; DestDir: "{app}\bin\64bit"; Flags: ignoreversion restartreplace
Source: "..\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs

[Code]
// Helper function to check if OBS Studio is installed on the computer
function IsOBSInstalled(): Boolean;
var
  ObsExePath1, ObsExePath2: String;
begin
  Result := False;

  // 1. Check standard 64-bit Program Files location
  ObsExePath1 := ExpandConstant('{commonpf64}\obs-studio\bin\64bit\obs64.exe');
  ObsExePath2 := ExpandConstant('{commonpf}\obs-studio\bin\64bit\obs64.exe');

  if FileExists(ObsExePath1) or FileExists(ObsExePath2) then
  begin
    Result := True;
    Exit;
  end;

  // 2. Check Windows Registry (Uninstall keys and Software keys)
  if RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OBS Studio') or
     RegKeyExists(HKLM, 'SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\OBS Studio') or
     RegKeyExists(HKCU, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OBS Studio') or
     RegKeyExists(HKLM, 'SOFTWARE\OBS Studio') or
     RegKeyExists(HKCU, 'SOFTWARE\OBS Studio') then
  begin
    Result := True;
    Exit;
  end;
end;

// Helper function to check if OBS Studio is currently running
function IsOBSRunning(): Boolean;
var
  ResultCode: Integer;
begin
  Result := False;
  if Exec('cmd.exe', '/c tasklist /FI "IMAGENAME eq obs64.exe" | find /i "obs64.exe"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Result := (ResultCode = 0);
  end;
end;

// Called automatically before the setup wizard is displayed
function InitializeSetup(): Boolean;
var
  Retries: Integer;
begin
  // First, verify OBS Studio installation
  if not IsOBSInstalled() then
  begin
    MsgBox('OBS Studio was not found on this computer.' + #13#10 + #13#10 +
           'Please download and install OBS Studio (version 28.0 or newer) from https://obsproject.com before installing the MobCam plugin.',
           mbCriticalError, MB_OK);
    Result := False;
    Exit;
  end;

  // Second, check if OBS Studio is running and prompt user to close it
  Retries := 0;
  while IsOBSRunning() and (Retries < 5) do
  begin
    if MsgBox('OBS Studio is currently running.' + #13#10 + #13#10 +
              'Please close OBS Studio before continuing so plugin files can be updated cleanly.' + #13#10 + #13#10 +
              'Click Retry after closing OBS, or Cancel to abort.',
              mbConfirmation, MB_RETRYCANCEL) = IDCANCEL then
    begin
      Result := False;
      Exit;
    end;
    Inc(Retries);
  end;

  if IsOBSRunning() then
  begin
    MsgBox('OBS Studio is still running. Setup cannot continue while OBS is active.',
           mbError, MB_OK);
    Result := False;
    Exit;
  end;

  Result := True;
end;

// Auto-register MobCam Controls dock in OBS Studio user.ini
procedure RegisterOBSBrowserDock();
var
  ObsDir, IniPath: String;
  CurrentDocks: String;
  NewDocks: String;
begin
  ObsDir := ExpandConstant('{userappdata}\obs-studio');
  IniPath := ObsDir + '\user.ini';
  if DirExists(ObsDir) or FileExists(IniPath) then
  begin
    CurrentDocks := GetIniString('BasicWindow', 'ExtraBrowserDocks', '', IniPath);
    if Pos('localhost:4752', CurrentDocks) = 0 then
    begin
      if (CurrentDocks = '') or (CurrentDocks = '[]') then
        NewDocks := '[{"title":"MobCam Controls","url":"http://localhost:4752/"}]'
      else if (Length(CurrentDocks) > 1) and (CurrentDocks[Length(CurrentDocks)] = ']') then
      begin
        NewDocks := Copy(CurrentDocks, 1, Length(CurrentDocks) - 1) + ',{"title":"MobCam Controls","url":"http://localhost:4752/"}]';
      end
      else
        NewDocks := '[{"title":"MobCam Controls","url":"http://localhost:4752/"}]';
      
      SetIniString('BasicWindow', 'ExtraBrowserDocks', NewDocks, IniPath);
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    RegisterOBSBrowserDock();
  end;
end;
