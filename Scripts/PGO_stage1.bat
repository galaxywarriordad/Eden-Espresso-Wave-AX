@echo off
setlocal enabledelayedexpansion
title Eden Emulator Pipeline - Stage 1 (Enterprise Instrumented Target Generation)

chcp 65001 >nul
set "PYTHONIOENCODING=utf-8"
for /f %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Add-Type -AssemblyName System.Windows.Forms;" ^
    "$w_code = '[DllImport(\"user32.dll\")] public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);';" ^
    "Add-Type -MemberDefinition $w_code -Name WWindowUtil -Namespace Win32 >$null;" ^
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

call "%SCRIPT_DIR%resolve_ax_tools.bat"
if !errorlevel! neq 0 exit /b 1

set "VSCMD_ARG_TGT_ARCH=x64"
set "VSCMD_ARG_HOST_ARCH=x64"
set "TARGET_CPU_ARCH=Coffee Lake"
set "TARGET_GPU_VENDOR=NVIDIA"
set "TARGET_GPU_ARCH=Maxwell"
set "PIPELINE_STAGE=Stage1_Production"

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

if not defined BASE_GAME_DIR set "BASE_GAME_DIR=J:\Switch"
if not defined SSD_STAGING_DIR set "SSD_STAGING_DIR=E:\updates"
echo [STATUS] Evaluating dual-storage target path layouts...
if not exist "%BASE_GAME_DIR%" echo [WARN] Mechanical archive directory (%BASE_GAME_DIR%) not mounted - continuing build.
if not exist "%SSD_STAGING_DIR%" mkdir "%SSD_STAGING_DIR%" 2>nul

cd /d "%PROJ_DIR%"

echo [STATUS] Executing synchronous workspace purge sequence...
powershell -NoProfile -ExecutionPolicy Bypass -File "%AX_TOOLS_DIR%\clean_all.ps1" -ProjDir "%PROJ_DIR%" -Wait
if !errorlevel! neq 0 exit /b 1

echo [STATUS] Verifying host hardware topology compliance...
powershell -NoProfile -ExecutionPolicy Bypass -File "%AX_TOOLS_DIR%\check_hardware.ps1" -TargetApp eden -RepoRoot "%PROJ_DIR%"
if !errorlevel! neq 0 exit /b !errorlevel!

echo [STATUS] Running shader macro harmonization...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%macro_update.ps1"
if !errorlevel! neq 0 exit /b 1

echo [STATUS] Running pre-build SPIR-V shader validation gate...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%shader_validate.ps1"
if !errorlevel! neq 0 exit /b 1

set "BUILD_DIR=%PROJ_DIR%\build_stage1"
set "VERBOSE_LOG=%PROJ_DIR%\build_stage1_verbose.log"
set "PGO_OUT_DIR=%LLVM_PROJ_DIR%/pgo_data"

for /f "tokens=*" %%r in ('"C:\Program Files\LLVM\bin\clang-cl.exe" -print-resource-dir') do set "CLANG_RES_DIR=%%r"
set "CLANG_PROF_LIB_DIR=%CLANG_RES_DIR%\lib\windows"

cls
if not defined LIVE_BUFFERS set "LIVE_BUFFERS=0"
if not defined LIVE_UNDERRUNS set "LIVE_UNDERRUNS=0"
if not defined LIVE_QSV_FRM set "LIVE_QSV_FRM=0"

echo                      ^(   ^(   ^(
echo                       ^)   ^)   ^)             PGO Phase 1 ^(Instrumentation Mode^)
echo                  ._________________.
echo                 /  ###############  \         --------------------------------------------
echo                /   ###############   \---\      === Eden ESPRESSO WAVE [AX] v1.35 ===
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
echo  [STAGE 1 - ENTERPRISE PRODUCTION] Injecting LLVM Instrumentation Hooks
echo =======================================================================

taskkill /f /im ninja.exe >nul 2>&1
taskkill /f /im cmake.exe >nul 2>&1
taskkill /f /im clang-cl.exe >nul 2>&1
taskkill /f /im lld-link.exe >nul 2>&1

if exist "%VERBOSE_LOG%" del /f /q "%VERBOSE_LOG%" >nul 2>&1
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%PROJ_DIR%\pgo_data" mkdir "%PROJ_DIR%\pgo_data"

echo =================================================== > "%VERBOSE_LOG%"
echo  EDEN EMULATOR PIPELINE DIAGNOSTIC REPORT          >> "%VERBOSE_LOG%"
echo =================================================== >> "%VERBOSE_LOG%"
echo Timestamp           : %DATE% %TIME%                 >> "%VERBOSE_LOG%"
echo Target Pipeline Phase: %PIPELINE_STAGE%             >> "%VERBOSE_LOG%"
echo Host System CPU     : %TARGET_CPU_ARCH%             >> "%VERBOSE_LOG%"
echo Host System GPU     : %TARGET_GPU_VENDOR% %TARGET_GPU_ARCH% >> "%VERBOSE_LOG%"
echo --------------------------------------------------- >> "%VERBOSE_LOG%"

powershell -NoProfile -ExecutionPolicy Bypass -Command "Get-CimInstance Win32_Processor | Select-Object Name, Description, NumberOfCores | Out-String | Out-File -FilePath '%VERBOSE_LOG%' -Encoding Ascii -Append" 2>nul
for /f "tokens=*" %%i in ('"C:\Program Files\LLVM\bin\clang-cl.exe" --version 2^>^&1') do echo LLVM Core Version   : %%i >> "%VERBOSE_LOG%"
echo --------------------------------------------------- >> "%VERBOSE_LOG%"

cd /d "%PROJ_DIR%"
set "OPT_FLAGS=/MD /O3 /Ob2 /Oi /Ot /Gw /Gy /DNDEBUG /D_WIN32_WINNT=0x0A00 /DWINVER=0x0A00 /D_HAS_CXX17=1 /fp:precise /std:c++20 /Zm2000 -Qunused-arguments -Xclang -faligned-allocation -D_CRT_ALIGNMENT=32 -march=skylake -fprofile-generate=%PGO_OUT_DIR% -clang:-Wno-profile-instr-out-of-date -Xclang -fcxx-exceptions"
set "C_OPT_FLAGS=/MD /O3 /Ob2 /Oi /Ot /Gw /Gy /DNDEBUG /D_WIN32_WINNT=0x0A00 /fp:precise -Qunused-arguments -march=skylake -fprofile-generate=%PGO_OUT_DIR%"
set "LINKER_FLAGS=/IGNORE:4006 /IGNORE:4221 /errorlimit:20 /libpath:\"%CLANG_PROF_LIB_DIR%\" /WHOLEARCHIVE:clang_rt.profile-x86_64.lib /DEFAULTLIB:msvcrt.lib /DEFAULTLIB:msvcprt.lib /DEFAULTLIB:ucrt.lib /DEFAULTLIB:bufferoverflowu.lib /DEFAULTLIB:mincore.lib /DEFAULTLIB:ws2_32.lib /OPT:REF /OPT:ICF ws2_32.lib"

echo [STATUS] Configuring Instrumented CMake Rules via Ninja...
powershell -NoProfile -Command "cmake -G Ninja -Wno-unused-cli -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER='C:/Program Files/LLVM/bin/clang-cl.exe' -DCMAKE_CXX_COMPILER='C:/Program Files/LLVM/bin/clang-cl.exe' -DCMAKE_LINKER='C:/Program Files/LLVM/bin/lld-link.exe' -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DCMAKE_CXX_FLAGS_RELEASE='%OPT_FLAGS%' -DCMAKE_C_FLAGS_RELEASE='%C_OPT_FLAGS%' -DCMAKE_EXE_LINKER_FLAGS_RELEASE='%LINKER_FLAGS%' -DOPUS_FIXED_POINT=OFF -DOPUS_FLOAT_APPROX=OFF -DBUILD_TESTING=OFF -S '%PROJ_DIR%' -B '%BUILD_DIR%' 2>&1 | Tee-Object -FilePath '%VERBOSE_LOG%' -Append"

cd /d "%BUILD_DIR%"
if not exist "%BUILD_DIR%\build.ninja" goto :cmake_fail

echo [STATUS] Launching live instrumentation build engine loop...
for /f "usebackq tokens=1" %%p in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "Add-Type -AssemblyName System.Windows.Forms; $p = Start-Process powershell -ArgumentList '-NoProfile','-NoExit','-ExecutionPolicy','Bypass','-File','\"%AX_TOOLS_DIR%\track_telemetry.ps1\"','-ProcessName','eden','-ConfigPath','\"%PROJ_DIR%\ax_engine.ini\"','-OutputJsonPath','\"%PROJ_DIR%\build_stage_telemetry_metrics.json\"' -WindowStyle Normal -PassThru; $w_code = '[DllImport(\''user32.dll'\')] public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);'; Add-Type -MemberDefinition $w_code -Name WUtil -Namespace Win32 >$null; Start-Sleep -Milliseconds 400; $h = (Get-Process -Name powershell | Where-Object { $_.MainWindowTitle -match 'TELEMETRY' -or $_.Id -eq $p.Id }).MainWindowHandle; if ($h -ne [IntPtr]::Zero) { [Win32.WUtil]::MoveWindow($h, [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width - 850, 40, 820, 480, $true) >$null }; $p.Id"`) do (set "TELEMETRY_PID=%%p")

powershell -NoProfile -Command "ninja -j 4 2>&1 | Tee-Object -FilePath '%VERBOSE_LOG%' -Append"
if not exist "%BUILD_DIR%\bin\eden.exe" goto :ninja_fail

if exist "%PROJ_DIR%\ax_engine.ini" copy /Y "%PROJ_DIR%\ax_engine.ini" "%BUILD_DIR%\bin\ax_engine.ini" >nul
if exist "%PROJ_DIR%\eden_engine.ini" copy /Y "%PROJ_DIR%\eden_engine.ini" "%BUILD_DIR%\bin\eden_engine.ini" >nul
if exist "%PROJ_DIR%\src\audio_core\FlexASIO.toml" copy /Y "%PROJ_DIR%\src\audio_core\FlexASIO.toml" "%BUILD_DIR%\bin\FlexASIO.toml" >nul

echo [STATUS] Running post-build telemetry regression check...
powershell -NoProfile -ExecutionPolicy Bypass -File "%AX_TOOLS_DIR%\check_regression.ps1" -RepoRoot "%PROJ_DIR%" -BaselineJson "%PROJ_DIR%\baseline_telemetry.json" -CurrentJson "%PROJ_DIR%\build_stage_telemetry_metrics.json"
if !errorlevel! neq 0 goto :ninja_fail

echo.
echo -----------------------------------------------------------------------
echo [STATUS] Instrumented tracing framework loaded successfully.
echo -----------------------------------------------------------------------
echo %ESC%[r
goto :exit_pipeline

:cmake_fail
echo %ESC%[r
echo [@=FATAL] SYSTEM GENERATION ERROR - CMake target rules configuration dropped out.
goto :error_exit

:ninja_fail
echo %ESC%[r
echo [@=FATAL] COMPILATION ERROR - Instrumented native target compilation failed.
goto :error_exit

:error_exit
if defined TELEMETRY_PID taskkill /f /pid %TELEMETRY_PID% >nul 2>&1
taskkill /f /fi "WINDOWTITLE eq *ASYNCHRONOUS PIPELINE TELEMETRY DAEMON*" >nul 2>&1
if defined CI_RUNNER_ACTIVE ( exit /b 1 ) else ( pause & exit /b 1 )

:exit_pipeline
if defined TELEMETRY_PID taskkill /f /pid %TELEMETRY_PID% >nul 2>&1
taskkill /f /fi "WINDOWTITLE eq *ASYNCHRONOUS PIPELINE TELEMETRY DAEMON*" >nul 2>&1
exit /b 0