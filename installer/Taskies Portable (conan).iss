; -- Install Taskies --

#define TaskiesVersion "0.3.5"

[Setup]
AppName=Taskies
AppVersion={#TaskiesVersion}
AppContact=szymonwelgus at gmail dot com
AppCopyright=Copyright (C) 2026 Szymon Welgus
AppPublisher=Szymon Welgus
AppId={{1FCF0393-F4E3-488E-9B39-4DDD442FD555}
AppVerName=Taskies {#TaskiesVersion}
DefaultDirName={userappdata}\Taskies
DefaultGroupName=Taskies
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
OutputDir=Portable
OutputBaseFilename=Taskies.{#TaskiesVersion}.Portable
LicenseFile=License
DisableWelcomePage=no
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Portable installation settings
CreateUninstallRegKey=no
Uninstallable=no
PrivilegesRequired=lowest
DisableProgramGroupPage=yes

[Files]
Source: "Taskies.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "mswu/*"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "bz2.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "bzip2.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "c_rehash.pl"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "curl-config"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "date-tz.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "fmt.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "libcrypto-3-x64.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "libcurl.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "libexpat.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "libjpeg-9.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "liblzma.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "libpng16.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "libssl-3-x64.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "lzmadec.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "lzmainfo.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "openssl.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "pcre2-8.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "pcre2-16.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "pcre2-32.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "pcre2-config"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "pcre2grep.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "pcre2-posix.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "spdlog.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "sqlite3.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "sqlite3.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "tiff.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "wcurl"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "wxbase32u_vc_x64_custom.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "wxmsw32u_core_vc_x64_custom.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "wxmsw32u_aui_vc_x64_custom.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "wxrc.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "xz.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "xzdec.exe"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "zlib1.dll"; DestDir: "{userappdata}"; Flags: ignoreversion
Source: "taskies.toml"; DestDir: "{userappdata}\Taskies"; Flags: onlyifdoesntexist
Source: "res\install-wizard.svg"; DestDir: "{userappdata}\res"; Flags: ignoreversion

[Icons]
Name: "{group}\Taskies"; Filename: "{userappdata}\Taskies.exe"

[Messages]
WelcomeLabel1=Welcome to the Taskies Portable Install Wizard

