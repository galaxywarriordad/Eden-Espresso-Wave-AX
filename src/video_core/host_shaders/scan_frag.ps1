Get-ChildItem -Recurse -Include *.frag | ForEach-Object {
    $path = $_.FullName
    $content = Get-Content $path
    # Search for numeric macro definitions
    $numericDefines = $content | Select-String -Pattern '#define\s+\w+\s+\d+'
    # Search for #version directives not in first 3 lines (possible inside includes)
    $versionLines = $content | Select-String -Pattern '^#version' | Where-Object { $_.LineNumber -gt 3 }
    # Search for macros defined without preceding ifndef guards 
    # (simple heuristic: #define preceded by less than 3 lines of #ifndef)
    # Note: this requires a bit more script logic to parse properly, basic version below:
    $lines = $content
    for ($i=0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -match '#define\s+\w+') {
            $foundIfndef = $false
            for ($j = [Math]::Max(0, $i-3); $j -lt $i; $j++) {
                if ($lines[$j] -match '#ifndef') { $foundIfndef = $true; break }
            }
            if (-not $foundIfndef) {
                Write-Output "${path}: Possible unguarded macro definition at line $($i+1): $($lines[$i])"
            }
        }
    }
    # Output warnings
    if ($numericDefines) {
        Write-Output "File $path contains numeric macro definitions:"
        $numericDefines | ForEach-Object { Write-Output "  $_" }
    }
    if ($versionLines) {
        Write-Output "File $path contains #version directive after first 3 lines:"
        $versionLines | ForEach-Object { Write-Output "  $_" }
    }
}
