@echo off
setlocal enabledelayedexpansion

set "ROOT_DIR=src\video_core\host_shaders"
set "TMP_FILE=%TEMP%\frag_tmp.txt"

for /r "%ROOT_DIR%" %%f in (*.frag) do (
    echo Processing %%f...
    setlocal enabledelayedexpansion
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
                set "line=%%l"
                echo !line! | findstr /r "^#version" >nul
                if errorlevel 1 (
                    echo !line!
                )
                rem skip original #version line
            ) else (
                if "!foundExtension!"=="0" (
                    echo #extension GL_GOOGLE_include_directive : enable
                    set "foundExtension=1"
                    set "line=%%l"
                    echo !line! | findstr /r "^#extension" >nul
                    if errorlevel 1 (
                        echo !line!
                    )
                    rem skip original #extension line
                ) else (
                    echo %%l
                )
            )
        )
    )

    move /Y "%TMP_FILE%" "%%f" >nul
    endlocal
)

echo Done updating #version and #extension directives.
