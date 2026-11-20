# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

param(
    [string]$LogDir = "$env:APPDATA\eden\log",
    [string]$ExePath = "$PSScriptRoot\..\build_stage0\bin\eden.exe"
)

$ErrorActionPreference = "Continue"

Write-Host "=======================================================================" -ForegroundColor Cyan
Write-Host " EDEN ESPRESSO WAVE [AX] - SINGLE-WINDOW RUN & LIVE TRIAGE ([TL])" -ForegroundColor Cyan
Write-Host " Watching: $LogDir" -ForegroundColor Cyan
Write-Host " Triage will auto-exit back to Configurator Hub when eden.exe closes." -ForegroundColor Yellow
Write-Host "=======================================================================" -ForegroundColor Cyan

$CandidateLogs = @(
    (Join-Path $LogDir "eden_log.txt"),
    (Join-Path $LogDir "eden_current.log"),
    "$env:APPDATA\eden\logs\eden_log.txt",
    "$env:APPDATA\eden\logs\eden_current.log"
)

# 0. Terminate any lingering background eden.exe processes & enforce ax-sdk silicon profile
Stop-Process -Name "eden" -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 300
& "$PSScriptRoot\reset_eden_config.ps1"

# 1. Clear stale session logs first so old runs are never replayed
Write-Host "[CLEAR] Purging previous session logs..." -ForegroundColor DarkCyan
foreach ($Path in $CandidateLogs) {
    if (Test-Path $Path) {
        try {
            Remove-Item -Path $Path -Force -ErrorAction Stop
        } catch {
            Clear-Content -Path $Path -Force -ErrorAction SilentlyContinue
        }
    }
}

# 2. Auto-launch eden.exe via explorer.exe so it runs with standard desktop user Vulkan/ICD privileges
$EdenProc = Get-Process -Name "eden" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($null -eq $EdenProc) {
    $ResolvedExe = (Resolve-Path -Path $ExePath -ErrorAction SilentlyContinue).Path
    if ($null -ne $ResolvedExe) {
        Write-Host "[LAUNCH] Starting $ResolvedExe via desktop shell..." -ForegroundColor Green
        Start-Process -FilePath "explorer.exe" -ArgumentList "`"$ResolvedExe`""
        for ($i = 0; $i -lt 15; $i++) {
            Start-Sleep -Milliseconds 200
            $EdenProc = Get-Process -Name "eden" -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($null -ne $EdenProc) { break }
        }
    } else {
        Write-Host "[WARN] $ExePath not found. Waiting for manual emulator launch..." -ForegroundColor Yellow
    }
} else {
    Write-Host "[ATTACH] Attached to running eden.exe (PID: $($EdenProc.Id))" -ForegroundColor Green
}

# 3. Wait for fresh log file creation
$TargetLog = $null
while ($null -eq $TargetLog) {
    foreach ($Path in $CandidateLogs) {
        if (Test-Path $Path) {
            $TargetLog = $Path
            break
        }
    }
    if ($null -eq $TargetLog) {
        if ($null -ne $EdenProc -and $EdenProc.HasExited) {
            Write-Host "[EXIT] eden.exe closed before creating a log file." -ForegroundColor Yellow
            exit 0
        }
        Start-Sleep -Milliseconds 200
    }
}

Write-Host "[ACTIVE] Streaming live session from $TargetLog ..." -ForegroundColor Green

$FilterPattern = "Arch_Maxwell|BAR1|VramShield|ax::|topology|underrun|EPIPE|Loading Shaders|Applet|DEVICE_LOST|<Error>|<Critical>|AX SHIELD|CLOSED-GAP|Render_Vulkan|PhysicalDevice"

function Write-TriageLine {
    param([string]$Text)
    if ($Text -match "<Error>|<Critical>|DEVICE_LOST|underrun|EPIPE") {
        Write-Host $Text -ForegroundColor Red
    } elseif ($Text -match "Arch_Maxwell|BAR1|VramShield|ax::|AX SHIELD|CLOSED-GAP") {
        Write-Host $Text -ForegroundColor Green
    } else {
        Write-Host $Text -ForegroundColor Yellow
    }
}

$FileStream = [System.IO.File]::Open($TargetLog, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
$Reader = New-Object System.IO.StreamReader($FileStream)

$LastSig = ""
$RepeatCount = 0

try {
    while ($true) {
        if ($FileStream.Length -lt $FileStream.Position) {
            $FileStream.Seek(0, [System.IO.SeekOrigin]::Begin) | Out-Null
            $Reader.DiscardBufferedData()
        }

        $Line = $Reader.ReadLine()
        if ($null -ne $Line) {
            if ($Line -match $FilterPattern) {
                $Sig = $Line -replace '^\[\s*\d+\.\d+\]\s*', ''
                if ($Sig -eq $LastSig) {
                    $RepeatCount++
                    continue
                }
                if ($RepeatCount -gt 0) {
                    Write-Host "  ... [suppressed $RepeatCount identical repeats]" -ForegroundColor DarkGray
                    $RepeatCount = 0
                }
                $LastSig = $Sig
                Write-TriageLine -Text $Line
            }
        } else {
            $EdenCheck = Get-Process -Name "eden" -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($null -eq $EdenCheck) {
                Start-Sleep -Milliseconds 250
                while ($null -ne ($FlushLine = $Reader.ReadLine())) {
                    if ($FlushLine -match $FilterPattern) {
                        Write-TriageLine -Text $FlushLine
                    }
                }
                if ($RepeatCount -gt 0) {
                    Write-Host "  ... [suppressed $RepeatCount identical repeats]" -ForegroundColor DarkGray
                }
                Write-Host "=======================================================================" -ForegroundColor Cyan
                Write-Host "[DONE] eden.exe exited. Returning to Configurator Hub..." -ForegroundColor Green
                break
            }
            Start-Sleep -Milliseconds 100
        }
    }
} finally {
    $Reader.Close()
    $FileStream.Close()
}
