$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build'
$srcDir = Join-Path $repoRoot 'src'
$qtRoot = 'C:\Qt\6.9.1\mingw_64'

$vsCmake = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$qtCmake = 'C:\Qt\Tools\CMake_64\bin\cmake.exe'

if (Test-Path $vsCmake) {
    $cmakeExe = $vsCmake
} elseif (Test-Path $qtCmake) {
    $cmakeExe = $qtCmake
} else {
    throw 'cmake.exe not found. Please install Visual Studio CMake tools or Qt CMake tools.'
}

if (!(Test-Path "$qtRoot\bin\mingw32-make.exe")) {
    throw "MinGW not found: $qtRoot"
}

if (Test-Path $buildDir) {
    Write-Host "Removing existing build directory: $buildDir"
    Remove-Item -Path $buildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $buildDir | Out-Null

$env:PATH = "$qtRoot\bin;$env:PATH"
Write-Host "Using CMake: $cmakeExe"
Write-Host "Using Qt/MinGW: $qtRoot"

& $cmakeExe -S $srcDir -B $buildDir -G 'MinGW Makefiles' `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_C_COMPILER="$qtRoot\bin\gcc.exe" `
  -DCMAKE_CXX_COMPILER="$qtRoot\bin\g++.exe" `
  -DCMAKE_MAKE_PROGRAM="$qtRoot\bin\mingw32-make.exe" `
  -DQt6_DIR="$qtRoot\lib\cmake\Qt6"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE"
}

$threads = [Environment]::ProcessorCount
Write-Host "Building with $threads threads..."
& $cmakeExe --build $buildDir --parallel $threads

if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE"
}

Write-Host 'Build completed successfully.'
