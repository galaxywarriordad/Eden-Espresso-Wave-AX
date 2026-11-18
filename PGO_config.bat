@echo off
setlocal enabledelayedexpansion
title Eden Espresso Wave [AX] Configurator

chcp 65001 >nul
set "PYTHONIOENCODING=utf-8"
for /f %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

set "SCRIPT_DIR=%~dp0"
set "PROJ_DIR=%SCRIPT_DIR%"
if "%PROJ_DIR:~-1%"=="\" set "PROJ_DIR=%PROJ_DIR:~0,-1%"

:: Run hardware check PowerShell from Scripts\ and capture output to temp log
set "HW_CHECK_LOG=%TEMP%\eden_hw_check.log"
if exist "%SCRIPT_DIR%Scripts\check_hardware.ps1" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%Scripts\check_hardware.ps1" > "%HW_CHECK_LOG%" 2>nul
) else (
    echo [WARN] Scripts\check_hardware.ps1 not found > "%HW_CHECK_LOG%"
)

:: Initialize status vars with default text
set "CORE0_STATUS=Unknown"
set "CORE1_STATUS=Unknown"
set "CORE5_STATUS=Unknown"

for /F "tokens=*" %%A in ('type "%HW_CHECK_LOG%"') do (
    echo %%A | findstr /C:"Host Controller" >nul
    if !errorlevel! == 0 set "CORE0_STATUS=%%A"
    echo %%A | findstr /C:"GTX 980 Ti" >nul
    if !errorlevel! == 0 set "CORE1_STATUS=%%A"
    echo %%A | findstr /C:"NVMe" >nul
    if !errorlevel! == 0 set "CORE5_STATUS=%%A"
)

:menu_loop
cls
echo                      (   (   (
echo                       )   )   )             Eden Espresso Wave [AX] Configurator
echo                  ._________________.
echo                 /  ###############  \         --------------------------------------------
echo                /   ###############   \---\      === Configurator Hub v1.3 ===
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
echo =======================================================================
echo  Please select stage or utility:
echo    [0]   PGO Stage 0 - High Velocity Production Linkage
echo    [0LD] Stage 0LD   - Lean Diagnostics Mode
echo    [1]   Stage 1     - Enterprise Instrumented Target Generation
echo    [2]   Stage 2     - Profile-Guided Optimization (Production Optimizer)
echo    [CA]  Clean All   - Absolute Workspace ^& Cache Purge (clean_all.ps1)
echo =======================================================================
echo.

set "CHOICE_STAGE="
set /p "CHOICE_STAGE=Enter target stage [0 / 0LD / 1 / 2 / CA]: "

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
if /i "%CHOICE_STAGE%"=="1" (
    set "TARGET_NAME=Stage 1 - Instrumentation"
    set "TARGET_SCRIPT=PGO_stage1.bat"
)
if /i "%CHOICE_STAGE%"=="2" (
    set "TARGET_NAME=Stage 2 - Optimisation"
    set "TARGET_SCRIPT=PGO_stage2.bat"
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
        echo [STATUS] Executing clean_all.ps1 workspace purge suite...
        powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%Scripts\clean_all.ps1"
        pause
        goto :menu_loop
    )
    echo [STATUS] Launching %TARGET_SCRIPT%...
    call "%SCRIPT_DIR%Scripts\%TARGET_SCRIPT%"
) else (
    echo [STATUS] Operation cancelled by user.
    echo [%DATE% %TIME%] User cancelled operation >> "%PROJ_DIR%\build_session.log"
    pause
    goto :menu_loop
)
exit /b 0
