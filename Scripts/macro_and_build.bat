@echo off
setlocal

echo [STATUS] Cleaning workspace via clean_all.ps1...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0clean_all.ps1"
if errorlevel 1 (
    echo [ERROR] Clean workspace failed.
    exit /b 1
)

echo [STATUS] Running batch macro update...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0macro_update.ps1"
if errorlevel 1 (
    echo [ERROR] Macro update failed.
    exit /b 1
)

echo [STATUS] Running Stage 0LD build...
call "%~dp0PGO_stage0LD.bat"
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo [STATUS] Running post-build validation...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0check_regression.ps1"
if errorlevel 1 (
    echo [ERROR] Performance regression detected.
    exit /b 1
)

echo [STATUS] Build and validation complete.
exit /b 0
