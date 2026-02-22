@echo off
chcp 65001 > nul
setlocal
echo @echo off > "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
echo echo Restoring environment >> "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
for %%v in (PATH OPENSSL_MODULES) do (
    set foundenvvar=
    for /f "delims== tokens=1,2" %%a in ('set') do (
        if /I "%%a" == "%%v" (
            echo set "%%a=%%b">> "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
            set foundenvvar=1
        )
    )
    if not defined foundenvvar (
        echo set %%v=>> "%~dp0/deactivate_conanrunenv-debug-x86_64.bat"
    )
)
endlocal


set "PATH=C:\Users\szymon.welgus\.conan2\p\b\date5c34fe86de0a4\p\bin;C:\Users\szymon.welgus\.conan2\p\b\libcu2c1653454b506\p\bin;C:\Users\szymon.welgus\.conan2\p\b\opens222d0551f3e37\p\bin;C:\Users\szymon.welgus\.conan2\p\b\sqlitb8379c7c849b2\p\bin;C:\Users\szymon.welgus\.conan2\p\b\spdlo073b37a5f6165\p\bin;C:\Users\szymon.welgus\.conan2\p\b\fmt36238934bb2c3\p\bin;C:\Users\szymon.welgus\.conan2\p\b\wxwida8e97a3d955ce\p\bin;C:\Users\szymon.welgus\.conan2\p\b\wxwida8e97a3d955ce\p\lib\vc_x64_dll;C:\Users\szymon.welgus\.conan2\p\b\libpn38f9f9033ae0a\p\bin;C:\Users\szymon.welgus\.conan2\p\b\libti7c3d555162e27\p\bin;C:\Users\szymon.welgus\.conan2\p\b\xz_utfce91a38d9b4c\p\bin;C:\Users\szymon.welgus\.conan2\p\b\libjp2e54a8b41627f\p\bin;C:\Users\szymon.welgus\.conan2\p\b\expat0c4464e0540d8\p\bin;C:\Users\szymon.welgus\.conan2\p\b\pcre2dedd3daf86cc1\p\bin;C:\Users\szymon.welgus\.conan2\p\b\zlibce981b299fbd3\p\bin;C:\Users\szymon.welgus\.conan2\p\b\bzip252a7cbf785c24\p\bin;%PATH%"
set "OPENSSL_MODULES=C:\Users\szymon.welgus\.conan2\p\b\opens222d0551f3e37\p\lib\ossl-modules"