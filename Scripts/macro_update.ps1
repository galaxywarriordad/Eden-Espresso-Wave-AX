param(
    [string]$ShaderDir = "$PSScriptRoot\..\src\video_core\host_shaders"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -Path $ShaderDir)) {
    Write-Error "Shader directory not found: $ShaderDir"
    exit 1
}

$wrapperFiles = Get-ChildItem -Path $ShaderDir -Recurse -Filter *.frag | Where-Object {
    ($_.Name -match '^(vulkan|opengl)_fidelityfx_fsr_.*\.frag$') -and
    ($_.Name -notin @('fidelityfx_fsr.frag', 'opengl_fidelityfx_fsr.frag'))
}

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

foreach ($file in $wrapperFiles) {
    $rawText = Get-Content -Path $file.FullName -Raw
    $lines = $rawText -split "\r?\n"
    $newLines = New-Object System.Collections.Generic.List[string]
    $seenMacros = @{}

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # 1. Standardize GLSL version
        if ($line -match '^\s*#version\s+') {
            $newLines.Add("#version 460 core")
            continue
        }

        # 2. Standardize include directive
        if ($line -match '^\s*#extension\s+GL_GOOGLE_include_directive') {
            $newLines.Add("#extension GL_GOOGLE_include_directive : enable")
            continue
        }

        # 3. Handle #ifndef <TARGET_MACRO> blocks idempotently
        if ($line -match '^\s*#ifndef\s+(USE_EASU|USE_RCAS|YUZU_USE_FP16|VERSION)\b') {
            $macro = $Matches[1]
            $newLines.Add($line)
            $newLines.Add("#define $macro 1")
            $seenMacros[$macro] = $true

            # Consume all existing #define lines inside this #ifndef block up to #endif
            while (($i + 1) -lt $lines.Count -and $lines[$i + 1] -notmatch '^\s*#endif\b') {
                $i++
            }
            continue
        }

        # 4. Normalize standalone #define <TARGET_MACRO> (bare or numeric) and deduplicate
        if ($line -match '^\s*#define\s+(USE_EASU|USE_RCAS|YUZU_USE_FP16|VERSION)(?:\s+(.*))?$') {
            $macro = $Matches[1]
            $rawVal = if ($Matches[2]) { $Matches[2].Trim() } else { "" }

            if (-not $seenMacros.ContainsKey($macro)) {
                $val = if ($rawVal -match '^\d+$') { $rawVal } else { "1" }
                $newLines.Add("#define $macro $val")
                $seenMacros[$macro] = $true
            }
            continue
        }

        $newLines.Add($line)
    }

    # Trim trailing blank lines and enforce clean LF line endings
    while ($newLines.Count -gt 0 -and $newLines[$newLines.Count - 1] -eq "") {
        $newLines.RemoveAt($newLines.Count - 1)
    }
    $updatedText = ($newLines -join "`n") + "`n"

    if ($updatedText -ne $rawText) {
        [System.IO.File]::WriteAllText($file.FullName, $updatedText, $utf8NoBom)
        Write-Output "[macro_update] Harmonized: $($file.Name)"
    }
}

Write-Output "[macro_update] Shader macro check complete ($($wrapperFiles.Count) wrappers verified)."
