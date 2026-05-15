; -- Install Taskies --

#define TaskiesVersion "0.3.4"

[Setup]
AppName=Taskies
AppVersion={#TaskiesVersion}
AppContact=szymonwelgus at gmail dot com
AppCopyright=Copyright (C) 2026 Szymon Welgus
AppPublisher=Szymon Welgus
AppId={{f106cae9-8ca1-45e2-bbad-49caf39a7a2c}
AppVerName=Taskies {#TaskiesVersion}
DefaultDirName={autopf}\Taskies
DefaultGroupName=Taskies
UninstallDisplayIcon={src}\Taskies.exe
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
OutputDir=Installer
OutputBaseFilename=Taskies-x64.{#TaskiesVersion}-Installer
LicenseFile=License
DisableWelcomePage=no
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
CreateUninstallRegKey=no
Uninstallable=no

UsedUserAreasWarning=no
; The above directive silences the following compiler warning:
;    Warning: The [Setup] section directive "PrivilegesRequired" is set to "admin" but per-user areas (HKCU,userdocs)
;    are used by the script. Regardless of the version of Windows, if the installation is administrative then you should
;    be careful about making any per-user area changes: such changes may not achieve what you are intending.
; Background info:
; This installer indeed asks for admin rights so the Taskies files can be copied to a place where they have at least
; a minimum layer of protection against changes, e.g. by malware, plus it handles things for the currently logged-in user
; in the registry (GUI wallet per-user options) and for some of the icons. For reasons too complicated to fully explain
; here this does not work as intended if the installing user does not have admin rights and has to provide the password
; of a user that does for installing: The settings of the admin user instead of those of the installing user are changed.
; Short of ripping out that per-user functionality the issue has no suitable solution. Fortunately, this will probably
; play a role in only in few cases as the first standard user in a Windows installation does have admin rights.
; So, for the time being, this installer simply disregards this problem.

[Files]
Source: "Taskies.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "mswu/*"; DestDir: "{src}"; Flags: ignoreversion
Source: "bz2.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "bzip2.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "c_rehash.pl"; DestDir: "{src}"; Flags: ignoreversion
Source: "curl-config"; DestDir: "{src}"; Flags: ignoreversion
Source: "date-tz.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "fmt.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "libcrypto-3-x64.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "libcurl.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "libexpat.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "libjpeg-9.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "liblzma.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "libpng16.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "libssl-3-x64.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "lzmadec.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "lzmainfo.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "openssl.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "pcre2-8.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "pcre2-16.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "pcre2-32.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "pcre2-config"; DestDir: "{src}"; Flags: ignoreversion
Source: "pcre2grep.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "pcre2-posix.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "spdlog.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "sqlite3.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "sqlite3.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "tiff.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "wcurl"; DestDir: "{src}"; Flags: ignoreversion
Source: "wxbase32u_net_vc_x64_custom.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "wxmsw32u_core_vc_x64_custom.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "wxmsw32u_aui_vc_x64_custom.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "wxrc.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "xz.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "xzdec.exe"; DestDir: "{src}"; Flags: ignoreversion
Source: "zlib1.dll"; DestDir: "{src}"; Flags: ignoreversion
Source: "taskies.toml"; DestDir: "{userappdata}\Taskies"; Flags: onlyifdoesntexist
Source: "res\install-wizard.svg"; DestDir: "{src}\res"; Flags: ignoreversion

[Icons]
Name: "{group}\Taskies"; Filename: "{src}\Taskies.exe"

[Messages]
WelcomeLabel1=Welcome to the Taskies Portable Install Wizard

