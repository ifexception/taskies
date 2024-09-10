; -- Install Taskies --

#define TaskiesVersion "0.1.2"
#define TaskiesLifeCycle "alpha"

[Setup]
AppName=Taskies
AppVersion={#TaskiesVersion}
AppContact=szymonwelgus at gmail dot com
AppCopyright=Copyright (C) 2024 Szymon Welgus
AppPublisher=Szymon Welgus
AppId={{f106cae9-8ca1-45e2-bbad-49caf39a7a2c}
AppVerName=Taskies {#TaskiesVersion}-{#TaskiesLifeCycle}
DefaultDirName={commonpf}\Taskies
DefaultGroupName=Taskies
UninstallDisplayIcon={app}\Taskies.exe
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
OutputDir=Installer
OutputBaseFilename=Taskies-x64.{#TaskiesVersion}-{#TaskiesLifeCycle}-Installer
LicenseFile=License
WindowResizable=no
DisableWelcomePage=no
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "Taskies.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "fmt.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "jpeg62.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "liblzma.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libpng16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "pcre2-16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "spdlog.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "sqlite3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "tiff.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "wxbase32u_vc_custom.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "wxmsw32u_core_vc_custom.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "taskies.toml"; DestDir: "{userappdata}\Taskies"; Flags: onlyifdoesntexist
Source: "lang\*"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\bell16x16.png"; DestDir: "{app}\res"; Flags: ignoreversion
Source: "res\bellnotification16x16.png"; DestDir: "{app}\res"; Flags: ignoreversion
Source: "res\wiztest.svg"; DestDir: "{app}\res"; Flags: ignoreversion

[Icons]
Name: "{group}\Taskies"; Filename: "{app}\Taskies.exe"

[Messages]
WelcomeLabel1=Welcome to the Taskies Installation Wizard

[UninstallDelete]
Type: filesandordirs; Name: "{userappdata}\Taskies\logs"
Type: files; Name: "{userappdata}\Taskies\taskies.toml"
Type: filesandordirs; Name: "{userappdata}\Taskies"
Type: filesandordirs; Name: "{userappdata}\Taskies"

[Registry]
Root: HKCU; Subkey: "Software\Taskies"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Taskies"; ValueType: dword; ValueName: "Installed"; ValueData: 1;  Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Taskies"; ValueType: string; ValueName: "Version"; ValueData: {#TaskiesVersion};  Flags: preservestringtype uninsdeletekey
Root: HKCR; Subkey: "Applications\Taskies.exe"; ValueType: none; ValueName: "IsHostApp"
