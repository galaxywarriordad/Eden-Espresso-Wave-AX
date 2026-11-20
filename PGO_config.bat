@echo off
setlocal enabledelayedexpansion
title Eden Espresso Wave [AX] Configurator
mode con: cols=135 lines=45

chcp 65001 >nul
set "PYTHONIOENCODING=utf-8"
for /f %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

set "SCRIPT_DIR=%~dp0"
set "PROJ_DIR=%SCRIPT_DIR%"
if "%PROJ_DIR:~-1%"=="\" set "PROJ_DIR=%PROJ_DIR:~0,-1%"

:: Resolve shared ax-sdk\tools directory
call "%SCRIPT_DIR%Scripts\resolve_ax_tools.bat" >nul 2>&1

:: Run hardware check PowerShell from ax-sdk\tools\ and capture output to temp log
set "HW_CHECK_LOG=%TEMP%\eden_hw_check.log"
if defined AX_TOOLS_DIR if exist "%AX_TOOLS_DIR%\check_hardware.ps1" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%AX_TOOLS_DIR%\check_hardware.ps1" -TargetApp eden -RepoRoot "%PROJ_DIR%" > "%HW_CHECK_LOG%" 2>nul
) else (
    echo [WARN] ax-sdk\tools\check_hardware.ps1 not found > "%HW_CHECK_LOG%"
)

:: Initialize status vars with default text
set "CORE0_STATUS=Unknown"
set "CORE1_STATUS=Unknown"
set "CORE5_STATUS=Unknown"

for /F "tokens=*" %%A in ('findstr /C:"Audio/Timing" "%HW_CHECK_LOG%" 2^>nul') do set "CORE0_STATUS=%%A"
for /F "tokens=*" %%A in ('findstr /C:"Vulkan Submit" "%HW_CHECK_LOG%" 2^>nul') do set "CORE1_STATUS=%%A"
for /F "tokens=*" %%A in ('findstr /C:"Aux Quarantine" "%HW_CHECK_LOG%" 2^>nul') do set "CORE5_STATUS=%%A"

:menu_loop
cls
echo                      (   (   (
echo                       )   )   )             Eden Espresso Wave [AX] Configurator
echo                  ._________________.
echo                 /  ###############  \         --------------------------------------------
echo                /   ###############   \---\      === Configurator Hub v1.35 ===
echo               /    ###############    \   \   --------------------------------------------
echo              /     ###############     /  /   CPU: Intel 9600k Coffee Lake Baseline (Skylake ISA)
echo              \     ###############    /  /    GPU: NVIDIA Maxwell GTX 980Ti
echo               \    ###############   /--/     GPU: Intel UHD Graphics 630
echo                \____________________/         AV Engine: Intel QSV Hardware Acceleration
echo                  `================'           Audio Link: FlexASIO Low-Latency Engine
echo                                               Audio Link: Steinberg Asio Ultra-Low Latency Engine
echo                                               --------------------------------------------

echo CPU ^& GPU ^& Storage IRQ Affinity:
echo   Audio IRQ Status:   !CORE0_STATUS!
echo   GPU IRQ Status:     !CORE1_STATUS!
echo   Storage IRQ Status: !CORE5_STATUS!

echo =======================================================================
echo  Eden Espresso Wave [AX] Configurator:
echo  [NOTE] Administrator privileges are required for Configurator Hub operations.
echo         [TL] automatically drops elevation via Explorer when launching Eden.
echo =======================================================================
echo  Please select stage or utility (Q to Quit):
echo  Standard
echo    [0]   PGO Stage 0 - High Velocity Production Linkage
echo    [1]   Stage 1     - Enterprise Instrumented Target Generation
echo    [2]   Stage 2     - Profile-Guided Optimization (Production Optimizer)
echo  Advanced
echo    [CA]  Clean All   - Absolute Workspace ^& Cache Purge (clean_all.ps1)
echo    [0LD] Stage 0LD   - Lean Diagnostics Mode (Clean Cold Build)
echo    [0I]  Stage 0I    - Incremental Delta Rebuild (~15s Ninja Pass)
echo    [TL]  Triage Log  - Live Runtime Log Filter (Run WITHOUT Admin rights)
echo =======================================================================
echo.

set "CHOICE_STAGE="
set /p "CHOICE_STAGE=Enter target stage [0 / 1 / 2 / CA / 0LD / 0I / TL / Q]: "

if /i "%CHOICE_STAGE%"=="Q" exit /b 0

:: Log user selection inside PROJ_DIR
echo [%DATE% %TIME%] User selected: %CHOICE_STAGE% >> "%PROJ_DIR%\build_session.log"

set "TARGET_NAME="
set "TARGET_SCRIPT="

if /i "%CHOICE_STAGE%"=="0" (
    set "TARGET_NAME=PGO Stage 0 - High Velocity"
    set "TARGET_SCRIPT=PGO_stage0.bat"
)
if /i "%CHOICE_STAGE%"=="0LD" (
    set "TARGET_NAME=Stage 0LD - Lean Diagnostics"
    set "TARGET_SCRIPT=PGO_stage0LD.bat"
)
if /i "%CHOICE_STAGE%"=="0I" (
    set "TARGET_NAME=Stage 0I - Incremental Delta Rebuild"
    set "TARGET_SCRIPT=PGO_stage0I.bat"
)
if /i "%CHOICE_STAGE%"=="1" (
    set "TARGET_NAME=Stage 1 - Instrumentation"
    set "TARGET_SCRIPT=PGO_stage1.bat"
)
if /i "%CHOICE_STAGE%"=="2" (
    set "TARGET_NAME=Stage 2 - Optimisation"
    set "TARGET_SCRIPT=PGO_stage2.bat"
)
if /i "%CHOICE_STAGE%"=="TL" (
    set "TARGET_NAME=Live Runtime Log Filter"
    set "TARGET_SCRIPT=_tail_log_runner"
)
if /i "%CHOICE_STAGE%"=="CA" (
    set "TARGET_NAME=Clean All Workspace Purge"
    set "TARGET_SCRIPT=_clean_runner"
)

if "%TARGET_SCRIPT%"=="" (
    echo [ERROR] Invalid selection. Aborting.
    pause
    goto :menu_loop
)

echo.
set "CONFIRM="
set /p "CONFIRM=Are you sure you want to start %TARGET_NAME%? [y/N]: "

if /i "%CONFIRM%"=="y" (
    echo [%DATE% %TIME%] User confirmed start: %TARGET_NAME% >> "%PROJ_DIR%\build_session.log"
    if "%TARGET_SCRIPT%"=="_clean_runner" (
        if not defined AX_TOOLS_DIR (
            echo [@=FATAL] ax-sdk\tools\ not found. Set AX_SDK_DIR or clone ax-sdk alongside this repo.
            pause
            goto :menu_loop
        )
        echo [STATUS] Executing clean_all.ps1 workspace purge suite...
        powershell -NoProfile -ExecutionPolicy Bypass -File "%AX_TOOLS_DIR%\clean_all.ps1" -ProjDir "%PROJ_DIR%" -Wait
        pause
        goto :menu_loop
    )
    if "%TARGET_SCRIPT%"=="_tail_log_runner" (
        echo [STATUS] Launching live filtered Eden runtime triage monitor...
        powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%Scripts\tail_eden_log.ps1"
        pause
        goto :menu_loop
    )
    echo [STATUS] Launching %TARGET_SCRIPT%...
    call "%SCRIPT_DIR%Scripts\%TARGET_SCRIPT%"
    pause
) else (
    echo [STATUS] Operation cancelled by user.
    echo [%DATE% %TIME%] User cancelled operation >> "%PROJ_DIR%\build_session.log"
    pause
    goto :menu_loop
)
exit /b 0
