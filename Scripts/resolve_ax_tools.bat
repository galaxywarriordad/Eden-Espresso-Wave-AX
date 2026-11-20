@echo off
set "AX_TOOLS_DIR="
if defined AX_SDK_DIR if exist "%%AX_SDK_DIR%%\tools\Get-AxProfile.ps1" (
    set "AX_TOOLS_DIR=%%AX_SDK_DIR%%\tools"
    goto :ax_tools_found
)
for %%%%D in (
    "%%PROJ_DIR%%\..\ax-sdk\tools"
    "%%PROJ_DIR%%\build_stage0\_deps\ax_sdk-src\tools"
    "%%PROJ_DIR%%\build_stage1\_deps\ax_sdk-src\tools"
    "%%PROJ_DIR%%\build_stage2\_deps\ax_sdk-src\tools"
) do (
    if exist "%%%%~D\Get-AxProfile.ps1" (
        set "AX_TOOLS_DIR=%%%%~fD"
        goto :ax_tools_found
    )
)
echo [@=FATAL] Unable to locate ax-sdk\tools\. Clone ax-sdk alongside this repo or set AX_SDK_DIR.
exit /b 1
:ax_tools_found
exit /b 0
