param(
    [string]$RootPath = "$PSScriptRoot\.."
)

Write-Output "Scanning shader wrappers and codec source files for macros..."

$fragFiles = Get-ChildItem -Path $RootPath -Recurse -Include *.frag, *.h, *.cpp | Where-Object {
    # Include host_shaders and host1x codecs folders primarily
    $_.FullName -match '\\host_shaders\\' -or $_.FullName -match '\\host1x\\codecs\\'
}

foreach ($file in $fragFiles) {
    $content = Get-Content $file.FullName
    foreach ($line in $content) {
        if ($line -match '^\s*#define\s+(USE_EASU|USE_RCAS|YUZU_USE_FP16|VERSION|.*FSR.*|OPUS_.*)') {
            Write-Output "Macro found in $($file.FullName): $line"
        }
    }
}

Write-Output "Macro search completed."
