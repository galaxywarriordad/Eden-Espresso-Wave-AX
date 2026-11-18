@echo off
setlocal enabledelayedexpansion

:: Base directory to start search (change this path to your shader root)
set "ROOT_DIR=src\video_core\host_shaders"

:: Temporary file for processing
set "TMP_FILE=%TEMP%\frag_tmp.txt"

for /r "%ROOT_DIR%" %%f in (*.frag) do (
    echo Processing %%f...
    setlocal enabledelayedexpansion
    set "firstLine="
    set "secondLine="
    set "foundVersion=0"
    set "foundExtension=0"

    > "%TMP_FILE%" (
        for /f "usebackq delims=" %%l in ("%%f") do (
            if "!foundVersion!"=="0" (
                echo #version 460 core
                set "foundVersion=1"
                if "!foundExtension!"=="0" (
                    echo #extension GL_GOOGLE_include_directive : enable
                    set "foundExtension=1"
                )
                rem Skip the original first line if it is a version or extension directive
                set "line=%%l"
                echo !line! | findstr /r "^#version" >nul && (
                    rem Skip original #version line
                ) else (
                    echo !line!
                )
            ) else if "!foundExtension!"=="0" (
                echo #extension GL_GOOGLE_include_directive : enable
                set "foundExtension=1"
                rem skip original #extension line if present
                set "line=%%l"
                echo !line! | findstr /r "^#extension" >nul && (
                    rem Skip original #extension line
                ) else (
                    echo !line!
                )
            ) else (
                echo %%l
            )
        )
    )

    move /Y "%TMP_FILE%" "%%f" >nul
    endlocal
)

echo Done updating #version and #extension directives.
