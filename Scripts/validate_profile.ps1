# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# =======================================================================
# EDEN PROFILE QUALITY GATE REPORT AND VALIDATION PROTOCOL (ENTERPRISE)
# =======================================================================
$ErrorActionPreference = "Stop"

if ($env:VSCMD_ARG_TGT_ARCH -ne "x64") {
    Write-Output "======================================================================="
    Write-Output " [@=FATAL] INVALID BUILD ENVIRONMENT ENCOUNTERED!"
    Write-Output "======================================================================="
    exit 1
}

$env:PATH = "C:\Program Files\LLVM\bin;" + $env:PATH
# Step up one level to target the proper project root directory
$ProjDir = Split-Path -Path $PSScriptRoot -Parent

$RawProfPath = Join-Path $ProjDir "pgo_data"
$Stage1Bin = Join-Path $ProjDir "build_stage1\bin\eden.exe"
$ValidationReport = Join-Path $ProjDir "profile_validation_report.log"

Write-Output "======================================================================="
Write-Output " [PROFILE MONITOR] Running Telemetry Profile Integrity Protocol"
Write-Output "======================================================================="

# Force atomic file initialization using secure native StreamWriter definitions
[System.IO.File]::WriteAllText($ValidationReport, "===================================================`r`n EDEN EMULATOR PROFILE QUALITY GATE REPORT`r`n===================================================`r`nEvaluation Date/Time : $((Get-Date).ToString('dd/MM/yyyy HH:mm:ss'))`r`n", [System.Text.Encoding]::UTF8)

$ProfFiles = Get-ChildItem -Path $RawProfPath -Filter "*.profraw" -ErrorAction SilentlyContinue
if (-not $ProfFiles) {
    Write-Output "[@=FATAL] No profile trace blocks detected inside - $RawProfPath"
    Add-Content -Path $ValidationReport -Value "[METADATA_START]`r`nGLOBAL_STATUS=FAILED`r`nFAULT_REASON=G1_FAULT: No profraw files located.`r`n[METADATA_END]"
    exit 1
}

Write-Output "[STATUS] Running Allocation Footprint Scan..."
$CorruptDetected = $false

foreach ($File in $ProfFiles) {
    $MetaBlock = "[METADATA_START]`r`nTARGET_PROFILE=$($File.Name)`r`nALLOCATION_BYTES=$($File.Length)"
    
    if ($File.Length -le 128) {  # Enterprise minimum operational size limit gate for LLVM indexing block frames
        Write-Output "  [STATUS] Detached Truncated File - $($File.Name) (Invalid Payload Weight)"
        $MetaBlock += "`r`nVALIDATION_STATUS=FAILED`r`nFAULT_REASON=Truncated or empty profile trace`r`n[METADATA_END]"
        $CorruptDetected = $true
    } else {
        Write-Output "  [STATUS] Allocation Space Clear - $($File.Name) [$($File.Length) Bytes]"
        $MetaBlock += "`r`nVALIDATION_STATUS=PASSED`r`n[METADATA_END]"
    }
    Add-Content -Path $ValidationReport -Value $MetaBlock
}

if ($CorruptDetected) {
    Write-Output "[@=FATAL] Profile generation pass detected truncated data frames. Halting stage 2."
    exit 1
}

if (-not (Test-Path $Stage1Bin)) {
    Write-Output "[@=FATAL] Instrumented reference binary not found at - $Stage1Bin"
    Add-Content -Path $ValidationReport -Value "[METADATA_START]`r`nSTAGE1_BINARY=MISSING`r`nGLOBAL_QUALITY_STATUS=FAILED`r`nFAULT_REASON=Missing absolute reference binary`r`n[METADATA_END]"
    exit 1
}
Write-Output "[STATUS] Running Deep Structural LLVM Counter Binding Verification..."

foreach ($File in $ProfFiles) {
    Write-Output "Checking structural sync for - $($File.Name)"
    Add-Content -Path $ValidationReport -Value "[METADATA_START]`r`nSTRUCTURAL_TARGET=$($File.Name)"
    
    # HARDENED NATIVE INVOCATION: Completely bypasses Invoke-Expression token injection faults
    $ShowArgs = @("show", "--summary", "--object=$Stage1Bin", $File.FullName)
    
    try {
        $ProfDataOutput = & llvm-profdata.exe $ShowArgs 2>&1
    } catch {
        Write-Output "[@=FATAL] HOOK SYMBOL MISMATCH ENCOUNTERED ON FILE - $($File.Name)"
        Add-Content -Path $ValidationReport -Value "SYNC_STATUS=CRITICAL_MISMATCH`r`n[METADATA_END]"
        exit 1
    }
    
    $HasFunctionsCount = $false
    foreach ($Line in $ProfDataOutput) {
        if ($Line -match "Number\s+of\s+functions:\s*([0-9]+)") {
            $FuncCount = $Matches[1]
            Write-Output "  [STATUS] Sync Map Confirmed - Number of functions: $FuncCount"
            Add-Content -Path $ValidationReport -Value "COUNTER_SYNC_FUNCTIONS=$FuncCount`r`nSYNC_STATUS=PASSED`r`n[METADATA_END]"
            $HasFunctionsCount = $true
            break
        }
    }
    
    if (-not $HasFunctionsCount) {
        Write-Output "  [STATUS] Missing valid tracking headers in telemetry file - $($File.Name)"
        Add-Content -Path $ValidationReport -Value "SYNC_STATUS=FAILED`r`nFAULT_REASON=Missing tracking header metrics`r`n[METADATA_END]"
        $CorruptDetected = $true
    }
}
if ($CorruptDetected) { exit 1 }

Write-Output ""
Write-Output "[STATUS] Performing Profiler Validation Pass..."

try {
    $OutProfData = Join-Path $RawProfPath "validation_gate_tmp.profdata"
    
    # FIXED: Leverage native argument indexing arrays to pass clean unescaped forward path string variables to LLVM
    $ArgList = @("merge", "-output=$OutProfData", "$RawProfPath/*.profraw")
    & llvm-profdata.exe $ArgList 2>&1 | Out-Null
    
    if (Test-Path $OutProfData) {
        Remove-Item -Path $OutProfData -Force
        Add-Content -Path $ValidationReport -Value "[METADATA_START]`r`nDRY_RUN_MERGE=PASSED`r`n[METADATA_END]"
    } else {
        throw "Merge pass generated no valid optimized master matrix output file."
    }
} catch {
    Write-Output "[@=FATAL] LLVM profile data engine validation flat test failed."
    Add-Content -Path $ValidationReport -Value "[METADATA_START]`r`nDRY_RUN_MERGE=FAILED`r`nFAULT_REASON=DRY_RUN_FAULT: Merge collision detected.`r`n[METADATA_END]"
    exit 1
}

Write-Output "[STATUS] Dry-run flattening verified. Matrix calculation sequence stable."
Write-Output "-----------------------------------------------------------------------"
Write-Output "[STATUS] Global Telemetry Profile Data Cleared for Stage 2 Production!"
Write-Output "-----------------------------------------------------------------------"

Add-Content -Path $ValidationReport -Value "[METADATA_START]`r`nGLOBAL_QUALITY_STATUS=EXCELLENT_STREAM_READY`r`n[METADATA_END]"
exit 0
