Get-ChildItem -Recurse -Include *.frag | ForEach-Object {
    $file = $_.FullName
    $lines = Get-Content $file

    $output = @()
    $insideGuard = $false

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Match macro define line
        if ($line -match '^\s*#define\s+(\w+)(\s+.*)?$') {
            $macroName = $matches[1]

            # Check if previous lines contain #ifndef for the same macro
            $guardFound = $false
            for ($j = [Math]::Max(0, $i-3); $j -lt $i; $j++) {
                if ($lines[$j] -match "#ifndef\s+$macroName") {
                    $guardFound = $true
                    break
                }
            }

            if (-not $guardFound) {
                # Insert #ifndef and #define wraps
                $output += "#ifndef $macroName"
                $output += $line
                $output += "#endif"
                continue
            }
        }

        # Wrap VERSION macros even if numeric
        if ($line -match '^\s*#define\s+VERSION\s+\d+') {
            # Check for ifndef VERSION within previous lines
            $versionGuardFound = $false
            for ($j = [Math]::Max(0, $i-3); $j -lt $i; $j++) {
                if ($lines[$j] -match '#ifndef\s+VERSION') {
                    $versionGuardFound = $true
                    break
                }
            }
            if (-not $versionGuardFound) {
                $output += "#ifndef VERSION"
                $output += $line
                $output += "#endif"
                continue
            }
        }

        $output += $line
    }

    # Overwrite file only if changes detected
    $newContent = $output -join "`r`n"
    $oldContent = $lines -join "`r`n"

    if ($newContent -ne $oldContent) {
        Write-Output "Updating $file"
        Set-Content -Path $file -Value $newContent
    }
}
