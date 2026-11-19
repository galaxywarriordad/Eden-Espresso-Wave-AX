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

cd /d "%PROJ_DIR%"

echo [STATUS] Verifying hardware topology and macro configuration...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%check_hardware.ps1"
if !errorlevel! neq 0 exit /b !errorlevel!

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%macro_update.ps1"
if errorlevel 1 (
    echo [ERROR] Macro update failed. Aborting.
    exit /b 1
)

set "BUILD_DIR=%PROJ_DIR%\build_stage2"
set "VERBOSE_LOG=%PROJ_DIR%\build_stage2_verbose.log"
set "PCO_PROFDATA=%LLVM_PROJ_DIR%/pgo_data/default.profdata"

if exist "%VERBOSE_LOG%" del /f /q "%VERBOSE_LOG%" >nul 2>&1
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

if exist "%SCRIPT_DIR%validate_profile.ps1" (
    echo [STATUS] Validating LLVM PGO profile counters via validate_profile.ps1...
    powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%validate_profile.ps1"
    if !errorlevel! neq 0 exit /b !errorlevel!
)
if not exist "%PROJ_DIR%\pgo_data\default.profdata" (
    echo [@=FATAL] PGO profile missing: %PROJ_DIR%\pgo_data\default.profdata — run Stage 1 sweep and llvm-profdata merge first.
    exit /b 1
)

set "OPT_FLAGS=/MD /O3 /Ob3 /Oi /Ot /Gw /Gy /DNDEBUG /D_WIN32_WINNT=0x0A00 /fp:fast /std:c++20 /Zm2000 -Qunused-arguments -Xclang -faligned-allocation -D_CRT_ALIGNMENT=32 -march=skylake -mavx2 -mfma -mbmi -mbmi2 -fprofile-use=%PCO_PROFDATA% -clang:-Wno-profile-instr-out-of-date -Xclang -fcxx-exceptions"
set "C_OPT_FLAGS=/MD /O3 /Ob3 /Oi /Ot /Gw /Gy /DNDEBUG /D_WIN32_WINNT=0x0A00 /fp:fast -Qunused-arguments -march=skylake -fprofile-use=%PCO_PROFDATA%"
set "LINKER_FLAGS=/IGNORE:4006 /IGNORE:4221 /errorlimit:20 /MAP:%BUILD_DIR%\eden_stage2.map /DEFAULTLIB:msvcrt.lib /DEFAULTLIB:msvcprt.lib /DEFAULTLIB:ucrt.lib /DEFAULTLIB:bufferoverflowu.lib /DEFAULTLIB:mincore.lib /DEFAULTLIB:ws2_32.lib /OPT:REF /OPT:ICF ws2_32.lib"

echo [STATUS] Configuring Final PGO Profile-Guided Optimized CMake Build via Ninja...
cmake -G Ninja -Wno-unused-cli -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe" -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe" -DCMAKE_LINKER="C:/Program Files/LLVM/bin/lld-link.exe" -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DCMAKE_CXX_FLAGS_RELEASE="%OPT_FLAGS%" -DCMAKE_C_FLAGS_RELEASE="%C_OPT_FLAGS%" -DCMAKE_EXE_LINKER_FLAGS_RELEASE="%LINKER_FLAGS%" -DOPUS_FIXED_POINT=OFF -DOPUS_FLOAT_APPROX=OFF -DBUILD_TESTING=OFF -S "%PROJ_DIR%" -B "%BUILD_DIR%" > "%VERBOSE_LOG%" 2>&1
if !errorlevel! neq 0 goto :cmake_fail

cd /d "%BUILD_DIR%"
if not exist "%BUILD_DIR%\build.ninja" goto :cmake_fail

echo [STATUS] Compiling PGO production release targets (-j 4)...
ninja -j 4 >> "%VERBOSE_LOG%" 2>&1
if !errorlevel! neq 0 goto :ninja_fail
if not exist "%BUILD_DIR%\bin\eden.exe" goto :ninja_fail

echo [STATUS] Running post-build telemetry regression check...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%check_regression.ps1"
if !errorlevel! neq 0 goto :ninja_fail

echo.
echo [STATUS] Engine linkage successful. Running distributor provisioning checks...
if not exist "%PROJ_DIR%\eden_engine.ini" (
    echo [STATUS] Local template profile missing. Generating fallback eden_engine.ini configuration layout...
    (
        echo [EngineTopology]
        echo EnableThreadAffinityMasks=true
        echo EnforceStrictCoreClamps=true
        echo MinCoresForIsolationSplit=6
        echo CoreRatio=2:3:1
        echo.
        echo [WindowsPlatformHacks]
        echo EnableMMCSSProAudioScaling=true
        echo HostTimingPriorityClass=HIGH
        echo VulkanDriverPriorityClass=HIGH
        echo GuestExecutionPriorityClass=ABOVENORMAL
    ) > "%PROJ_DIR%\eden_engine.ini"
)

echo [STATUS] Deploying configuration components downstream alongside binary targets...
copy /Y "%PROJ_DIR%\eden_engine.ini" "%BUILD_DIR%\bin\eden_engine.ini" >nul
if exist "%PROJ_DIR%\src\audio_core\FlexASIO.toml" (
    copy /Y "%PROJ_DIR%\src\audio_core\FlexASIO.toml" "%BUILD_DIR%\bin\FlexASIO.toml" >nul
    echo [STATUS] Pre-staged FlexASIO.toml deployment map finalized.
)

echo.
echo -----------------------------------------------------------------------
echo [STATUS] Engine optimization compilation pipeline verification successful.
echo -----------------------------------------------------------------------
echo %ESC%[r
choice /c YN /n /m " -> View final optimization feedback log traces? [Y/N]: "
if errorlevel 2 goto :exit_pipeline
if errorlevel 1 notepad "%VERBOSE_LOG%"

:exit_pipeline
exit /b 0

:cmake_fail
echo %ESC%[r
echo [@=FATAL] SYSTEM GENERATION ERROR - CMake configuration failed.
exit /b 1

:ninja_fail
echo %ESC%[r
echo [@=FATAL] COMPILATION ERROR - Stage 2 Optimization build compilation dropped out.
exit /b 1