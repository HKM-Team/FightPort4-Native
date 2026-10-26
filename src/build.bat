@echo off
setlocal
echo Compiling FightPort 4 with Visual Studio Build Tools...

set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist %VCVARS% (
    echo Error: vcvars64.bat not found at %VCVARS%
    exit /b 1
)

call %VCVARS%
cd /d "%~dp0"

echo Compiling resources...
rc.exe /nologo /fo app_msvc.res app.rc
if %ERRORLEVEL% neq 0 (
    echo Resource compilation failed.
    exit /b %ERRORLEVEL%
)

echo Compiling C code...
cl.exe /nologo /O2 /W3 /utf-8 /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE ^
    fightport4.c pkg_engine.c app_msvc.res ^
    /link /OUT:"..\FightPort4.exe" /SUBSYSTEM:WINDOWS ^
    comctl32.lib shell32.lib shlwapi.lib uxtheme.lib gdiplus.lib ole32.lib ^
    dwmapi.lib gdi32.lib user32.lib advapi32.lib msimg32.lib

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================================
    echo SUCCESS: FightPort4.exe built successfully in root folder!
    echo ========================================================
) else (
    echo Build failed with error code %ERRORLEVEL%
)

del /q app_msvc.res fightport4.obj pkg_engine.obj 2>nul
