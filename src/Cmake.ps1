$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Path $PSScriptRoot -Parent
$sourceDir = Join-Path $repoRoot "src"
$buildDir = Join-Path $repoRoot "build\legacy-cmake"

Write-Warning "StartTAP is not built through CMake. This script only builds the legacy C fallback targets."

if (Test-Path -LiteralPath $buildDir) {
    Remove-Item -LiteralPath $buildDir -Recurse -Force
}

New-Item -ItemType Directory -Path $buildDir | Out-Null

& cmake -S $sourceDir -B $buildDir
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& cmake --build $buildDir --config Release
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Legacy fallback build completed successfully."
