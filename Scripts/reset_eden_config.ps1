# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

param(
    [string]$ConfigPath  = "$env:APPDATA\eden\config\qt-config.ini",
    [string]$ProfileFile = "",
    [string]$CpuProfile  = "",
    [string]$GpuProfile  = ""
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\Get-AxProfile.ps1"

Write-Host "=======================================================================" -ForegroundColor Cyan
Write-Host " EDEN ESPRESSO WAVE [AX] - DECLARATIVE SILICON CONFIG ENFORCER (v2.1)" -ForegroundColor Cyan
Write-Host "=======================================================================" -ForegroundColor Cyan

# 1. Terminate any lingering eden.exe process so it cannot overwrite qt-config.ini
$Running = Get-Process -Name "eden" -ErrorAction SilentlyContinue
if ($null -ne $Running) {
    Write-Host "[CLEANUP] Terminating lingering eden.exe process(es)..." -ForegroundColor Yellow
    $Running | Stop-Process -Force -ErrorAction SilentlyContinue
    Start-Sleep -Milliseconds 400
}

if (-not (Test-Path $ConfigPath)) {
    Write-Host "[ERROR] qt-config.ini not found at $ConfigPath" -ForegroundColor Red
    exit 1
}

# 2. Resolve canonical silicon profile from ax-sdk
$Hw = Get-AxHardwareProfile -ProfileFile $ProfileFile -CpuProfile $CpuProfile -GpuProfile $GpuProfile
$ActiveCpu = $Hw.Cpu
$ActiveGpu = $Hw.Gpu

Write-Host "[SOURCE]  $($Hw.ProfileFile)" -ForegroundColor DarkGray
Write-Host "[CPU]     $($Hw.CpuName) ($($Hw.PhysicalCores)C/$($Hw.LogicalCores)T) -> '$($Hw.CpuKey)' [$($ActiveCpu.topology_mode)]" -ForegroundColor DarkCyan
Write-Host "[MASKS]   Audio: $($ActiveCpu.masks.audio_irq) | Vulkan: $($ActiveCpu.masks.vulkan_submit) | JIT: $($ActiveCpu.masks.jit_workers) | Storage: $($ActiveCpu.masks.storage_aux)" -ForegroundColor DarkCyan

if (-not $Hw.SmtCompliant) {
    Write-Host "[WARN]    Hyper-Threading/SMT appears ENABLED ($($Hw.LogicalCores) logical > $($ActiveCpu.expected_logical_cores) expected). Disable HT in BIOS for deterministic latency!" -ForegroundColor Yellow
}

Write-Host "[GPU]     $($Hw.GpuNames) -> '$($Hw.GpuKey)' [$($ActiveGpu.architecture)]" -ForegroundColor DarkCyan
Write-Host "[LIMITS]  VRAM Ceiling: $($ActiveGpu.vram_ceiling_gib) GiB | Reap: $($ActiveGpu.vram_reap_trigger_gib) GiB | BAR1: $($ActiveGpu.bar1_staging_mib) MiB" -ForegroundColor Green

# 3. Merge common_qt_config + GPU-specific qt_config overrides
$Overrides = @{}
foreach ($Entry in $Hw.Matrix.common_qt_config.PSObject.Properties) {
    if ($Entry.Name -like "_comment*") { continue }
    $Overrides[$Entry.Name] = [string]$Entry.Value
    $Overrides["$($Entry.Name)\default"] = "false"
}
foreach ($Entry in $ActiveGpu.qt_config.PSObject.Properties) {
    if ($Entry.Name -like "_comment*") { continue }
    $Overrides[$Entry.Name] = [string]$Entry.Value
    $Overrides["$($Entry.Name)\default"] = "false"
}

# 4. Rewrite qt-config.ini deterministically while preserving user paths & controller mappings
$Lines = Get-Content -Path $ConfigPath
$CurrentSection = ""

$UpdatedLines = foreach ($Line in $Lines) {
    if ($Line -match '^\[(.+)\]$') {
        $CurrentSection = $Matches[1]
        $Line
    } elseif ($Line -match '^([^=]+)=(.*)$') {
        $Key = $Matches[1]
        $Lookup = "$CurrentSection|$Key"
        if ($Overrides.ContainsKey($Lookup)) {
            "$Key=$($Overrides[$Lookup])"
        } else {
            $Line
        }
    } else {
        $Line
    }
}

Set-Content -Path $ConfigPath -Value $UpdatedLines -Encoding UTF8
Write-Host "[PASS] Enforced $($Overrides.Count / 2) settings ('$($Hw.CpuKey)' + '$($Hw.GpuKey)') into qt-config.ini!" -ForegroundColor Green
Write-Host "=======================================================================" -ForegroundColor Cyan
