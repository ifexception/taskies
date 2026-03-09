@echo off
chcp 65001 > nul
setlocal
echo @echo off > "%~dp0/deactivate_conanrunenv-release-x86_64.bat"
echo echo Restoring environment >> "%~dp0/deactivate_conanrunenv-release-x86_64.bat"
for %%v in (PATH OPENSSL_MODULES) do (
    set foundenvvar=
    for /f "delims== tokens=1,2" %%a in ('set') do (
        if /I "%%a" == "%%v" (
            echo set "%%a=%%b">> "%~dp0/deactivate_conanrunenv-release-x86_64.bat"
            set foundenvvar=1
        )
    )
    if not defined foundenvvar (
        echo set %%v=>> "%~dp0/deactivate_conanrunenv-release-x86_64.bat"
    )
)
endlocal


set "PATH=C:\Users\szymon.welgus\.conan2\p\date73845b282d91f\p\bin;C:\Users\szymon.welgus\.conan2\p\libcu16bf9db8b8f3f\p\bin;C:\Users\szymon.welgus\.conan2\p\opens4e0faa71393f8\p\bin;C:\Users\szymon.welgus\.conan2\p\sqlit64327c3d1a7ec\p\bin;C:\Users\szymon.welgus\.conan2\p\spdlobc1e5ffd0c357\p\bin;C:\Users\szymon.welgus\.conan2\p\fmtb023e645abce4\p\bin;C:\Users\szymon.welgus\.conan2\p\wxwidd771f727ad894\p\bin;C:\Users\szymon.welgus\.conan2\p\wxwidd771f727ad894\p\lib\vc_x64_dll;C:\Users\szymon.welgus\.conan2\p\libpn9f90f107f9bb0\p\bin;C:\Users\szymon.welgus\.conan2\p\libti183ecb9d6939b\p\bin;C:\Users\szymon.welgus\.conan2\p\xz_utd6ffb843a5f7c\p\bin;C:\Users\szymon.welgus\.conan2\p\libjp9100cdb58e0c1\p\bin;C:\Users\szymon.welgus\.conan2\p\expat4f41b1aeed640\p\bin;C:\Users\szymon.welgus\.conan2\p\pcre27aa246bc7105f\p\bin;C:\Users\szymon.welgus\.conan2\p\zlibe7061fb113fae\p\bin;C:\Users\szymon.welgus\.conan2\p\bzip2217ece51953bc\p\bin;%PATH%"
set "OPENSSL_MODULES=C:\Users\szymon.welgus\.conan2\p\opens4e0faa71393f8\p\lib\ossl-modules"