param(
    [string]$ShaderDir = "$PSScriptRoot\..\src\video_core\host_shaders",
    [string]$ExternalsDir = "$PSScriptRoot\..\externals"
)

$glslang = "C:\VulkanSDK\1.4.363.0\Bin\glslangValidator.exe"

if (-not (Test-Path -Path $glslang)) {
    Write-Host "[shader_validate] glslangValidator not found at: $glslang" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path -Path $ShaderDir)) {
    Write-Host "[shader_validate] Shader directory not found: $ShaderDir" -ForegroundColor Red
    exit 1
}

# Auto-locate ffx_a.h under externals\
$ffxHeader = Get-ChildItem -Path $ExternalsDir -Recurse -Filter "ffx_a.h" -ErrorAction SilentlyContinue | Select-Object -First 1
$includeArgs = @()
if ($ffxHeader) {
    $includeArgs += "-I$($ffxHeader.DirectoryName)"
    Write-Host "[shader_validate] Found FSR headers at: $($ffxHeader.DirectoryName)" -ForegroundColor Cyan
} else {
    Write-Host "[shader_validate] WARNING: ffx_a.h not found under $ExternalsDir" -ForegroundColor Yellow
}

$shaderFiles = Get-ChildItem -Path $ShaderDir -Recurse -Filter *.frag | Where-Object {
    ($_.Name -notin @('fidelityfx_fsr.frag', 'opengl_fidelityfx_fsr.frag')) -and
    ($_.Name -notmatch '^opengl_')
}

$errors = 0

foreach ($file in $shaderFiles) {
    $ErrorActionPreference = "Continue"
    $result = & $glslang -V --quiet @includeArgs -DUseUniformBlock=1 --target-env spirv1.3 $file.FullName 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[shader_validate] FAILED: $($file.Name)" -ForegroundColor Red
        $result | ForEach-Object { Write-Host "  $_" }
        $errors++
    }
}

if ($errors -gt 0) {
    Write-Host "[shader_validate] $errors shader(s) failed validation." -ForegroundColor Red
    exit 1
} else {
    Write-Host "[shader_validate] All $($shaderFiles.Count) Vulkan fragment shaders validated successfully." -ForegroundColor Green
    exit 0
}
