@echo off
setlocal enabledelayedexpansion
title Eden Emulator Pipeline - Stage 0 PGO (High-Velocity Production Linkage)

chcp 65001 >nul
set "PYTHONIOENCODING=utf-8"
for /f %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Add-Type -AssemblyName System.Windows.Forms;" ^
    "$w_code = '[DllImport(\"user32.dll\")] public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);';" ^
    "Add-Type -TypeDefinition $w_code -Name WWindowUtil -Namespace Win32 >$null;" ^
    "Start-Sleep -Milliseconds 150;" ^
    "$h = (Get-Process -Id $PID).MainWindowHandle;" ^
    "if ($h -ne [IntPtr]::Zero) {" ^
    "    $screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds;" ^
    "    $halfW = [int]($screen.Width / 2);" ^
    "    [Win32.WWindowUtil]::MoveWindow($h, 0, 0, $halfW, $screen.Height, $true) >$null;" ^
    "}"

set "SCRIPT_DIR=%~dp0"
for %%i in ("%SCRIPT_DIR%..") do set "PROJ_DIR=%%~fi"
if "%PROJ_DIR:~-1%"=="\" set "PROJ_DIR=%PROJ_DIR:~0,-1%"
set "LLVM_PROJ_DIR=%PROJ_DIR:\=/%"

set "VSCMD_ARG_TGT_ARCH=x64"
set "VSCMD_ARG_HOST_ARCH=x64"
set "TARGET_CPU_ARCH=Coffee Lake"
set "TARGET_GPU_VENDOR=NVIDIA"
set "TARGET_GPU_ARCH=Maxwell"
set "PIPELINE_STAGE=PGO_Stage0_HighVelocity"

echo [STATUS] Auditing system privilege security layer tokens...
net session >nul 2>&1
if !errorlevel! neq 0 (
    echo [@=FATAL] INSUFFICIENT SYSTEM ADMINISTRATIVE PRIVILEGES DETECTED! Please run as Administrator.
    if defined CI_RUNNER_ACTIVE exit /b 1
    pause
    exit /b 1
)
echo [STATUS] Administrative context active. Security tokens authorized.

set "PATH=C:\Program Files\CMake\bin;C:\Program Files\LLVM\bin;C:\Program Files\ninja-win;C:\Program Files\Git\cmd;C:\Program Files\Git\bin;%PATH%"
set "QT_ENABLE_HIGHDPI_SCALING=1"
set "QT_SCALE_FACTOR_ROUNDING_POLICY=PassThrough"

set "BASE_GAME_DIR=J:\Switch"
set "SSD_STAGING_DIR=E:\updates"
echo [STATUS] Evaluating dual-storage target path layouts...
if not exist "%BASE_GAME_DIR%" echo [@=FATAL] MECHANICAL TARGET DIRECTORY UNREACHABLE - DRIVE MOUNT MISSING. & exit /b 1
if not exist "%SSD_STAGING_DIR%" mkdir "%SSD_STAGING_DIR%" 2>nul

cd /d "%PROJ_DIR%"

echo [STATUS] Verifying host hardware topology compliance...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%check_hardware.ps1"
if !errorlevel! neq 0 (
    set "HW_ERR=!errorlevel!"
    echo [@=FATAL] HARDWARE COMPLIANCE FIREWALL REJECTED HOST TOPOLOGY.
    if defined CI_RUNNER_ACTIVE exit /b !HW_ERR!
    pause
    exit /b !HW_ERR!
)

echo [STATUS] Running shader macro harmonization...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%macro_update.ps1"
if !errorlevel! neq 0 (
    echo [@=FATAL] Shader macro update failed.
    exit /b 1
)

echo [STATUS] Running pre-build SPIR-V shader validation gate...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%shader_validate.ps1"
if !errorlevel! neq 0 (
    echo [@=FATAL] SPIR-V shader validation failed.
    exit /b 1
)

set "BUILD_DIR=%PROJ_DIR%\build_stage0"
set "SUMMARY_LOG=%PROJ_DIR%\build_stage0_summary.log"
set "NINJA_OUT=%BUILD_DIR%\ninja_last_run.tmp"

cls

echo                      ^(   ^(   ^(
echo                       ^)   ^)   ^)             PGO Phase 0 ^(High-Velocity Production Mode^)
echo                  ._________________.
echo                 /  ###############  \         --------------------------------------------
echo                /   ###############   \---\      === Eden ESPRESSO WAVE [AX] v1.3 ===
echo               /    ###############    \   \   --------------------------------------------
echo              /     ###############     /  /   CPU: Intel 9600k Coffee Lake Baseline ^(Skylake ISA^)
echo              \     ###############    /  /    GPU: NVIDIA Maxwell GTX 980Ti
echo               \    ###############   /--/     GPU: Intel UHD Graphics 630
echo                \____________________/         AV Engine: Intel QSV Hardware Acceleration
echo                  `================'           Audio Link: FlexASIO Low-Latency Engine
echo                                               Audio Link: Steinberg Asio Ultra-Low Latency Engine 
echo                                               --------------------------------------------

echo %ESC%[45;r%ESC%[45;1H
echo =======================================================================
echo  [STAGE 0 - HIGH-VELOCITY BUILD] Incremental Production Pass
echo =======================================================================

echo =================================================== > "%SUMMARY_LOG%"
echo  EDEN PIPELINE SUMMARY REPORT (STAGE 0)            >> "%SUMMARY_LOG%"
echo =================================================== >> "%SUMMARY_LOG%"
echo Start Timestamp      : %DATE% %TIME%                >> "%SUMMARY_LOG%"
echo Target Pipeline Phase: %PIPELINE_STAGE%            >> "%SUMMARY_LOG%"

set "OPT_FLAGS=/MD /EHsc /O3 /Ob3 /Oi /Ot /Gw /Gy /DNDEBUG /D_WIN32_WINNT=0x0A00 /fp:fast /std:c++20 /Zm2000 -Qunused-arguments -Xclang -faligned-allocation -D_CRT_ALIGNMENT=32 -march=skylake -clang:-Wno-nan-infinity-disabled -clang:-Wno-error=unused-but-set-variable -Xclang -fcxx-exceptions"
set "C_OPT_FLAGS=/MD /O3 /Ob3 /Oi /Ot /Gw /Gy /DNDEBUG /D_WIN32_WINNT=0x0A00 /fp:fast -Qunused-arguments -march=skylake"
set "LINKER_FLAGS=/IGNORE:4006 /IGNORE:4221 /errorlimit:20 /MAP:%BUILD_DIR%\eden_stage0.map /DEFAULTLIB:msvcrt.lib /DEFAULTLIB:msvcprt.lib /DEFAULTLIB:ucrt.lib /DEFAULTLIB:bufferoverflowu.lib /DEFAULTLIB:mincore.lib /DEFAULTLIB:ws2_32.lib /OPT:REF /OPT:ICF ws2_32.lib"

if not exist "%BUILD_DIR%\build.ninja" (
    echo [STATUS] Cold cache detected - generating Ninja build graph...
    echo CMake Mode           : Cold Configure >> "%SUMMARY_LOG%"
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cmake -G Ninja -Wno-unused-cli -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe" -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe" -DCMAKE_LINKER="C:/Program Files/LLVM/bin/lld-link.exe" -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DCMAKE_CXX_FLAGS_RELEASE="%OPT_FLAGS%" -DCMAKE_C_FLAGS_RELEASE="%C_OPT_FLAGS%" -DCMAKE_EXE_LINKER_FLAGS_RELEASE="%LINKER_FLAGS%" -DOPUS_FIXED_POINT=OFF -DOPUS_FLOAT_APPROX=OFF -DBUILD_TESTING=OFF -S "%PROJ_DIR%" -B "%BUILD_DIR%"
    if !errorlevel! neq 0 goto :triage_fail
) else (
    echo [STATUS] Reusing existing Ninja build graph for high-velocity delta compile...
    echo CMake Mode           : Incremental Reuse >> "%SUMMARY_LOG%"
)

cd /d "%BUILD_DIR%"
if not exist "%BUILD_DIR%\build.ninja" goto :triage_fail

echo [STATUS] Compiling delta targets (-j 4)...
ninja -j 4 > "%NINJA_OUT%" 2>&1
set "NINJA_RC=!errorlevel!"
type "%NINJA_OUT%"

if !NINJA_RC! neq 0 goto :ninja_fail
if not exist "%BUILD_DIR%\bin\eden.exe" goto :ninja_fail

echo End Timestamp        : %DATE% %TIME%                >> "%SUMMARY_LOG%"
echo Build Result         : PASS (eden.exe linked)      >> "%SUMMARY_LOG%"
for /f "tokens=*" %%l in ('findstr /r /c:"^\[[0-9]*/[0-9]*\]" "%NINJA_OUT%"') do set "LAST_NINJA_STEP=%%l"
if defined LAST_NINJA_STEP echo Final Ninja Step     : !LAST_NINJA_STEP! >> "%SUMMARY_LOG%"
del /q "%NINJA_OUT%" >nul 2>&1

if exist "%PROJ_DIR%\eden_engine.ini" copy /Y "%PROJ_DIR%\eden_engine.ini" "%BUILD_DIR%\bin\eden_engine.ini" >nul
if exist "%PROJ_DIR%\src\audio_core\FlexASIO.toml" copy /Y "%PROJ_DIR%\src\audio_core\FlexASIO.toml" "%BUILD_DIR%\bin\FlexASIO.toml" >nul

echo [PASS] Stage 0 linkage verification successful: %BUILD_DIR%\bin\eden.exe
echo [INFO] Summary written to: %SUMMARY_LOG%
if defined CI_RUNNER_ACTIVE goto :exit_pipeline

choice /c YN /n /m " -> Open build_stage0_summary.log? [Y/N]: "
if errorlevel 2 goto :exit_pipeline
if errorlevel 1 start "" notepad.exe "%SUMMARY_LOG%"

:exit_pipeline
echo %ESC%[r
echo [STATUS] Stage 0 pipeline complete.
exit /b 0

:triage_fail
echo End Timestamp        : %DATE% %TIME%                >> "%SUMMARY_LOG%"
echo Build Result         : FAIL (CMake Configure)      >> "%SUMMARY_LOG%"
echo %ESC%[r
echo [@=FATAL] SYSTEM GENERATION ERROR - CMake baseline configuration failed.
if not defined CI_RUNNER_ACTIVE if exist "%SUMMARY_LOG%" start "" notepad.exe "%SUMMARY_LOG%"
exit /b 1

:ninja_fail
echo End Timestamp        : %DATE% %TIME%                >> "%SUMMARY_LOG%"
echo Build Result         : FAIL (Compile / Link)       >> "%SUMMARY_LOG%"
echo --- Error Extract ---                              >> "%SUMMARY_LOG%"
if exist "%NINJA_OUT%" (
    findstr /i /c:"error:" /c:"FAILED:" /c:"lld-link: error" "%NINJA_OUT%" >> "%SUMMARY_LOG%"
    del /q "%NINJA_OUT%" >nul 2>&1
)
echo %ESC%[r
echo [@=FATAL] COMPILATION ERROR - Native link phase encountered structural build faults.
if not defined CI_RUNNER_ACTIVE if exist "%SUMMARY_LOG%" start "" notepad.exe "%SUMMARY_LOG%"
exit /b 1