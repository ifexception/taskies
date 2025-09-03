; -- Install Taskies --

#define TaskiesVersion "0.2.8"

[Setup]
AppName=Taskies
AppVersion={#TaskiesVersion}
AppContact=szymonwelgus at gmail dot com
AppCopyright=Copyright (C) 2025 Szymon Welgus
AppPublisher=Szymon Welgus
AppId={{f106cae9-8ca1-45e2-bbad-49caf39a7a2c}
AppVerName=Taskies {#TaskiesVersion}
DefaultDirName={autopf}\Taskies
DefaultGroupName=Taskies
UninstallDisplayIcon={app}\Taskies.exe
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
OutputDir=Installer
OutputBaseFilename=Taskies-x64.{#TaskiesVersion}-Installer
LicenseFile=License
DisableWelcomePage=no
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

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
Source: "Taskies.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "fmt.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "jpeg62.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "liblzma.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libpng16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libsharpyuv.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libwebp.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libwebpdecoder.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "libwebpdemux.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "pcre2-16.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "spdlog.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "sqlite3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "tiff.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "wxbase331u_vc_x64_custom.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "wxmsw331u_core_vc_x64_custom.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "taskies.toml"; DestDir: "{userappdata}\Taskies"; Flags: onlyifdoesntexist
Source: "lang\*"; DestDir: "{app}\lang"; Flags: ignoreversion
Source: "res\bell16x16.png"; DestDir: "{app}\res"; Flags: ignoreversion
Source: "res\bellnotification16x16.png"; DestDir: "{app}\res"; Flags: ignoreversion
Source: "res\install-wizard.svg"; DestDir: "{app}\res"; Flags: ignoreversion

[Icons]
Name: "{group}\Taskies"; Filename: "{app}\Taskies.exe"

[Messages]
WelcomeLabel1=Welcome to the Taskies Installation Wizard

[UninstallDelete]
Type: filesandordirs; Name: "{userappdata}\Taskies\logs"
Type: files; Name: "{userappdata}\Taskies\taskies.toml"
Type: files; Name: "{app}\fmt.dll";
Type: files; Name: "{app}\jpeg62.dll";
Type: files; Name: "{app}\liblzma.dll";
Type: files; Name: "{app}\libpng16.dll";
Type: files; Name: "{app}\pcre2-16.dll";
Type: files; Name: "{app}\spdlog.dll";
Type: files; Name: "{app}\sqlite3.dll";
Type: files; Name: "{app}\tiff.dll";
Type: files; Name: "{app}\wxbase32u_vc_x64_custom.dll";
Type: files; Name: "{app}\wxmsw32u_core_vc_x64_custom.dll";
Type: files; Name: "{app}\zlib1.dll";
Type: filesandordirs; Name: "{app}\lang";
Type: filesandordirs; Name: "{app}\res";


[Registry]
Root: HKCU; Subkey: "Software\Taskies"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Taskies"; ValueType: dword; ValueName: "Installed"; ValueData: 1;  Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Taskies"; ValueType: string; ValueName: "Version"; ValueData: {#TaskiesVersion};  Flags: preservestringtype uninsdeletekey
Root: HKCR; Subkey: "Applications\Taskies.exe"; ValueType: none; ValueName: "IsHostApp"
