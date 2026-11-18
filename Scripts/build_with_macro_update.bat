@echo off
setlocal enabledelayedexpansion

echo [STATUS] Cleaning workspace...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0clean_all.ps1"
if errorlevel 1 (
    echo [ERROR] Clean failed. Aborting.
    exit /b 1
)

echo [STATUS] Running macro update...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0macro_update.ps1"
if errorlevel 1 (
    echo [ERROR] Macro update failed. Aborting.
    exit /b 1
)

echo [STATUS] Validating shaders...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0shader_validate.ps1"
if errorlevel 1 (
    echo [ERROR] Shader validation failed. Aborting.
    exit /b 1
)

echo [STATUS] Starting Stage 0LD build...
call "%~dp0PGO_stage0LD.bat"
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo [STATUS] Running telemetry regression check...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0check_regression.ps1"
if errorlevel 1 (
    echo [ERROR] Regression detected.
    exit /b 1
)

echo [STATUS] Build and validation complete.
exit /b 0
