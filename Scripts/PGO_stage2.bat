@echo off
setlocal enabledelayedexpansion
title Eden Emulator Pipeline - Stage 2 (Enterprise Production Optimizer)

chcp 65001 >nul
set "PYTHONIOENCODING=utf-8"

for /f %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

set "SCRIPT_DIR=%~dp0"
for %%i in ("%SCRIPT_DIR%..") do set "PROJ_DIR=%%~fi"
if "%PROJ_DIR:~-1%"=="\" set "PROJ_DIR=%PROJ_DIR:~0,-1%"
set "LLVM_PROJ_DIR=%PROJ_DIR:\=/%"

set "VSCMD_ARG_TGT_ARCH=x64"
set "TARGET_CPU_ARCH=Coffee Lake"
set "TARGET_GPU_VENDOR=NVIDIA"
set "TARGET_GPU_ARCH=Maxwell"
set "PIPELINE_STAGE=Stage2_Production_Optimization"

net session >nul 2>&1
if !errorlevel! neq 0 (
    echo [@=FATAL] INSUFFICIENT SYSTEM ADMINISTRATIVE PRIVILEGES DETECTED!
    pause
    exit /b 1
)

set "PATH=C:\Program Files\CMake\bin;C:\Program Files\LLVM\bin;C:\Program Files\ninja-win;C:\Program Files\Git\cmd;C:\Program Files\Git\bin;%PATH%"
set "QT_ENABLE_HIGHDPI_SCALING=1"
set "QT_SCALE_FACTOR_ROUNDING_POLICY=PassThrough"

set "BASE_GAME_DIR=J:\Switch"
set "SSD_STAGING_DIR=E:\updates"
if not exist "%BASE_GAME_DIR%" echo [@=FATAL] MECHANICAL TARGET DIRECTORY UNREACHABLE - DRIVE MOUNT MISSING. & exit /b 1
if not exist "%SSD_STAGING_DIR%" mkdir "%SSD_STAGING_DIR%" 2>nul

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%clean_all.ps1"
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%check_hardware.ps1"

echo [STATUS] Running macro update script...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%macro_update.ps1"
if errorlevel 1 (
    echo [ERROR] Macro update failed. Aborting.
    exit /b 1
)

:: Insert your CMake configuration and Ninja build commands here

echo [STATUS] Running telemetry regression check...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%check_regression.ps1"
if errorlevel 1 (
    echo [ERROR] Regression detected. Aborting.
    exit /b 1
)

:: Additional post build code and cleanup as in your current script
if !errorlevel! neq 0 (if not exist "!BUILD_DIR!\bin\eden.exe" goto :ninja_fail)
if not exist "!BUILD_DIR!\bin\eden.exe" goto :ninja_fail

echo [STATUS] Running post-bake telemetry regression check...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%check_regression.ps1"
if !errorlevel! neq 0 goto :ninja_fail

echo.
echo [STATUS] Engine linkage successful. Running distributor provisioning checks...
if not exist "!PROJ_DIR!\eden_engine.ini" (
    echo [STATUS] Local template profile missing. Generating fallback eden_engine.ini configuration layout...
    (
        echo [EngineTopology]
        echo EnableThreadAffinityMasks=true
        echo EnforceStrictCoreClamps=true
        echo MinCoresForIsolationSplit=6
        echo.
        echo [WindowsPlatformHacks]
        echo EnableMMCSSProAudioScaling=true
    ) > "!PROJ_DIR!\eden_engine.ini"
)

echo [STATUS] Deploying configuration components downstream alongside binary targets...
copy /Y "!PROJ_DIR!\eden_engine.ini" "!BUILD_DIR!\bin\eden_engine.ini" >nul
if exist "!PROJ_DIR!\src\audio_core\FlexASIO.toml" (
    copy /Y "!PROJ_DIR!\src\audio_core\FlexASIO.toml" "!BUILD_DIR!\bin\FlexASIO.toml" >nul
    echo [STATUS] Pre-staged FlexASIO.toml deployment map finalized.
)

echo.
echo -----------------------------------------------------------------------
echo [STATUS] Engine optimization compilation pipeline verification successful.
echo -----------------------------------------------------------------------
echo %ESC%[r
choice /c YN /n /m " -> View final optimization feedback log traces? [Y/N]: "
if errorlevel 2 goto :exit_pipeline
if errorlevel 1 notepad "!VERBOSE_LOG!"

:exit_pipeline
if defined TELEMETRY_PID taskkill /f /pid !TELEMETRY_PID! >nul 2>&1
exit /b 0

:cmake_fail
echo %ESC%[r
echo [@=FATAL] SYSTEM GENERATION ERROR - CMake configuration failed.
if defined TELEMETRY_PID taskkill /f /pid !TELEMETRY_PID! >nul 2>&1
exit /b 1

:ninja_fail
echo %ESC%[r
echo [@=FATAL] COMPILATION ERROR - Stage 2 Optimization build compilation dropped out.
if defined TELEMETRY_PID taskkill /f /pid !TELEMETRY_PID! >nul 2>&1
exit /b 1
