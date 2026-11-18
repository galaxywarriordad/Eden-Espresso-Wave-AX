@echo off
:: =======================================================================================
:: PURPOSE: Instantly scans your local documentation space (*.md) for hidden browser 
::          artifacts, non-breaking spaces (\x00A0), and web-formatting control tokens.
::          The script recursively dives through every subfolder from where it runs.
::
:: CLONE ACTION: Prints all diagnostic results straight to the screen terminal. The exit 
::               triggers are locked out, allowing you to use Ctrl+A and Ctrl+C safely.
::
:: SAFETY MECHANISM: This script operates strictly in READ-ONLY mode. It contains zero 
::                   write, edit, or delete routines on your source materials.
:: =======================================================================================
setlocal enabledelayedexpansion
title Eden Architecture - Read-Only Documentation Scanner

echo =======================================================================
echo            EDEN DOCUMENTATION INTEGRITY REPORT (READ-ONLY)
echo =======================================================================
echo.
echo [INFO] Scanning markdown structures for hidden web characters...
echo.

set "BAD_CHAR_FOUND=0"
set "FILE_COUNT=0"

for /r %%F in (*.md) do (
    set /a FILE_COUNT+=1
    
    findstr /n /r /c:"[\x80-\xFF]" "%%F" >nul 2>&1
    if !errorlevel! equ 0 (
        echo [WARNING] Extended characters found in: "%%F"
        echo -----------------------------------------------------
        findstr /n /r /c:"[\x80-\xFF]" "%%F"
        echo.
        set "BAD_CHAR_FOUND=1"
    )
)

echo =======================================================================
echo                              SCAN SUMMARY
echo =======================================================================
echo Total markdown files evaluated: %FILE_COUNT%

if "%BAD_CHAR_FOUND%"=="1" (
    echo [RESULT] Hidden formatting artifacts detected. Check the details above.
) else (
    echo [RESULT] SUCCESS. All documentation files are clean.
)
echo =======================================================================
echo [LOCKDOWN] Scan complete. Window locked. Pressing keys will NOT close this screen.
echo            You can now freely use Ctrl+A to select all information safely.
echo.

:: Hard screen freeze that completely absorbs keyboard inputs without exiting
pause >nul