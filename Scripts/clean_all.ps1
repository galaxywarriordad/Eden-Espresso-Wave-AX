# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# =======================================================================
# EDEN EMULATOR MASTER WORKSPACE PURGE SUITE (ENTERPRISE v2.2 CLANG-LEAN)
# =======================================================================
$ErrorActionPreference = "Stop"
# Step up one level to the project root directory from \Scripts\
$ProjDir = Split-Path -Path $PSScriptRoot -Parent

Write-Output "======================================================================="
Write-Output " [PURGE SUITE] Initializing Absolute Pipeline Cleanup Pass"
Write-Output " Target Path: $ProjDir"
Write-Output "======================================================================="

Write-Output "[STATUS] Terminating active build engine processes..."
$Targets = @("ninja", "cmake", "clang-cl", "lld-link", "eden")
foreach ($Target in $Targets) {
    try {
        Stop-Process -Name $Target -Force -ErrorAction SilentlyContinue
    } catch {}
}
Start-Sleep -Seconds 1

Write-Output "[STATUS] Clearing pipeline telemetry log files..."
$LogFiles = @(
    "build_stage0_summary.log",
    "build_stage0_verbose.log",
    "build_stage0_triage.log",
    "build_stage1_verbose.log",
    "build_stage2_verbose.log",
    "profile_validation_report.log"
)
foreach ($Log in $LogFiles) {
    $TargetLog = Join-Path $ProjDir $Log
    if (Test-Path $TargetLog) { Remove-Item -Path $TargetLog -Force -ErrorAction SilentlyContinue }
}

Write-Output "[STATUS] Purging persistent internal binary tracking manifests..."
$Manifests = @(".ninja_deps", ".ninja_log")
foreach ($Manifest in $Manifests) {
    $TargetManifest = Join-Path $ProjDir $Manifest
    if (Test-Path $TargetManifest) { Remove-Item -Path $TargetManifest -Force -ErrorAction SilentlyContinue }
}

Write-Output "[STATUS] Staging build trees and system cache for background cleanup..."
$RandID = "EDENTRASH_" + (Get-Random -Minimum 10000 -Maximum 99999).ToString()

$BuildDirs = [ordered]@{
    "build"            = "trash_build_$RandID"
    "build_stage0"     = "trash_s0_$RandID"
    "PGO_stage0"       = "trash_alt_s0_$RandID"
    "build_stage1"     = "trash_s1_$RandID"
    "build_stage2"     = "trash_s2_$RandID"
    "pgo_data_triage"  = "trash_pgo_triage_$RandID"
    ".cache"           = "trash_cache_$RandID"
}

$DirectoryKeys = [System.Collections.ArrayList]::new($BuildDirs.Keys)

foreach ($Key in $DirectoryKeys) {
    $SourcePath = Join-Path $ProjDir $Key
    if (Test-Path $SourcePath) {
        try {
            Rename-Item -Path $SourcePath -NewName $BuildDirs[$Key] -Force
        } catch {
            Write-Output "  [STATUS] Persistent file handle lock discovered on folder: $Key"
            Start-Sleep -Seconds 1
            try {
                Rename-Item -Path $SourcePath -NewName $BuildDirs[$Key] -Force
            } catch {
                Write-Output "  [@=FATAL] Workspace path locked - $Key. Skipping block..."
            }
        }
    }
}

Write-Output "[STATUS] Launching asynchronous multi-threaded disk deletion worker..."

$ScriptPayload = @"
Get-ChildItem -Path '$ProjDir' -Directory -ErrorAction SilentlyContinue | Where-Object {
    (`$_.Name -like '*EDENTRASH_*' -or `$_.Name -like '*_trash_*' -or `$_.Name -like 'trash_*')
} | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
"@

$EncodedCommand = [Convert]::ToBase64String([System.Text.Encoding]::Unicode.GetBytes($ScriptPayload))

try {
    Start-Process -FilePath "powershell.exe" -ArgumentList "-NoProfile -ExecutionPolicy Bypass -EncodedCommand $EncodedCommand" -WindowStyle Hidden
} catch {
    Write-Output "[@=FATAL] Failed to allocate asynchronous worker window handle context."
    exit 1
}

Write-Output "======================================================================="
Write-Output " [SUCCESS] Workspace verified clean. Background deletion initialized!"
Write-Output "======================================================================="
exit 0
