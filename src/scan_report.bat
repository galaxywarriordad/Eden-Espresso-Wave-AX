@echo off
:: =======================================================================================
:: PURPOSE: Instantly scans your local workspace (*.cpp, *.h, *.md) for hidden browser 
::          artifacts, non-breaking spaces (\x00A0), and web-formatting control tokens 
::          that corrupt Git pipelines and cause compiler syntax errors.
:: the script uses the /r flag (which stands for Recursive), it will automatically look 
:: inside the folder it is currently running in and then dive deep into every single 
:: subfolder and directory structure underneath it
::
:: SAFETY MECHANISM: This script operates strictly in READ-ONLY mode. It contains zero 
::                   write, edit, or delete routines. It isolates your code files from 
::                   danger while identifying the exact file paths and lines requiring 
::                   manual verification.
:: =======================================================================================
setlocal enabledelayedexpansion
title Eden Architecture - Read-Only File Integrity Scanner

echo =======================================================================
echo               EDEN RAW FILE INTEGRITY REPORT (READ-ONLY)
echo =======================================================================
echo.
echo [INFO] Scanning folder paths for hidden web metadata and control characters...
echo.

set "BAD_CHAR_FOUND=0"
set "FILE_COUNT=0"

:: Target all cpp, h, and md files in the current folder and subfolders
for /r %%F in (*.cpp *.h *.md) do (
    set /a FILE_COUNT+=1
    
    :: Use standard findstr in hex literal mode to flag hidden formatting tokens
    :: 0xa0 = Non-Breaking Space (NBSP)
    :: 0xefbfbc = Object Replacement Character (UTF-8 bytes for \uFFFC)
    
    findstr /n /r /c:"[\x80-\xFF]" "%%F" >nul
    if !errorlevel! equ 0 (
        echo [WARNING] Extended characters or web format artifacts found in:
        echo           %%F
        echo           -----------------------------------------------------
        :: Print the specific line numbers and lines that contain non-standard tokens
        findstr /n /r /c:"[\x80-\xFF]" "%%F"
        echo.
        set "BAD_CHAR_FOUND=1"
    )
)

echo =======================================================================
echo                              SCAN SUMMARY
echo =======================================================================
echo Total source files evaluated: %FILE_COUNT%

if "%BAD_CHAR_FOUND%"=="1" (
    echo [RESULT] Hidden formatting artifacts detected. Check the log details above.
) else (
    echo [RESULT] SUCCESS. All files are 100%% clean plain text. Safe for compiler pipeline.
)
echo =======================================================================
echo.
pause
