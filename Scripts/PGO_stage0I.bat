@echo off
setlocal EnableDelayedExpansion

:: 1. Initialize Visual Studio x64 Native Tools & LLVM 23+ Toolchain Environment
set "PATH=C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;%PATH%"
if not defined VSCMD_VER (
    for %%V in (18 2026 2022) do (
        for %%E in (Community Professional Enterprise BuildTools) do (
            if exist "C:\Program Files\Microsoft Visual Studio\%%V\%%E\VC\Auxiliary\Build\vcvarsall.bat" (
                call "C:\Program Files\Microsoft Visual Studio\%%V\%%E\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
                goto :vs_ready
            )
        )
    )
)
:vs_ready

echo =======================================================================
echo  [STAGE 0I - INCREMENTAL] Fast Ninja Delta Compile ^& Link
echo =======================================================================

:: 1. Release any lingering elevated eden.exe lock before linking
taskkill /F /IM eden.exe /T >nul 2>&1

:: 2. Verify build_stage0 Ninja tree exists; otherwise bootstrap via Stage 0LD
if not exist "%~dp0..\build_stage0\build.ninja" (
    echo [WARN] No existing build_stage0 tree found. Falling back to Stage 0LD...
    call "%~dp0PGO_stage0LD.bat"
    exit /b %ERRORLEVEL%
)

set "LOG_FILE=%~dp0..\build_stage0_incremental.log"
echo [STATUS] Compiling modified C++20 targets via Ninja (-j 4)...
ninja -C "%~dp0..\build_stage0" -j 4 eden > "%LOG_FILE%" 2>&1
if %ERRORLEVEL% neq 0 (
    echo [@=FATAL] Incremental build failed. Check %LOG_FILE% for diagnostics.
    type "%LOG_FILE%"
    exit /b 1
)

:: 3. Re-stage runtime configs alongside updated binary
if exist "%~dp0..\ax_engine.ini" copy /Y "%~dp0..\ax_engine.ini" "%~dp0..\build_stage0\bin\ax_engine.ini" >nul 2>&1
if exist "%~dp0..\eden_engine.ini" copy /Y "%~dp0..\eden_engine.ini" "%~dp0..\build_stage0\bin\eden_engine.ini" >nul 2>&1
if exist "%~dp0..\src\audio_core\FlexASIO.toml" copy /Y "%~dp0..\src\audio_core\FlexASIO.toml" "%~dp0..\build_stage0\bin\FlexASIO.toml" >nul 2>&1

echo [PASS] Stage 0I incremental build successful: %~dp0..\build_stage0\bin\eden.exe
exit /b 0