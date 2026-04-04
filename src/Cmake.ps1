param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [ValidateSet('x64', 'x86')]
    [string]$Platform = 'x64',

    [switch]$NoRestore
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$solution = Join-Path $repoRoot 'start.sln'

if (!(Test-Path $solution)) {
    throw "Solution not found: $solution"
}

function Resolve-MSBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath | Select-Object -First 1
        if ($installPath) {
            $candidate = Join-Path $installPath 'MSBuild\Current\Bin\MSBuild.exe'
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    $msbuildCmd = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($msbuildCmd) {
        return $msbuildCmd.Source
    }

    throw 'MSBuild not found. Please install Visual Studio 2022 or Build Tools with MSBuild.'
}

function Resolve-NuGet {
    $nugetCmd = Get-Command nuget.exe -ErrorAction SilentlyContinue
    if ($nugetCmd) {
        return $nugetCmd.Source
    }

    $candidate = Join-Path $repoRoot 'nuget.exe'
    if (Test-Path $candidate) {
        return $candidate
    }

    throw 'nuget.exe not found. Please install NuGet CLI or place nuget.exe at repository root.'
}

$msbuildExe = Resolve-MSBuild
Write-Host "Using MSBuild: $msbuildExe"

if (-not $NoRestore) {
    $nugetExe = Resolve-NuGet
    Write-Host "Using NuGet: $nugetExe"
    & $nugetExe restore $solution
    if ($LASTEXITCODE -ne 0) {
        throw "NuGet restore failed with exit code $LASTEXITCODE"
    }
}

$threads = [Environment]::ProcessorCount
Write-Host "Building solution: $solution"
Write-Host "Configuration: $Configuration, Platform: $Platform"
& $msbuildExe $solution /m:$threads /t:Build "/p:Configuration=$Configuration" "/p:Platform=$Platform"

if ($LASTEXITCODE -ne 0) {
    throw "MSBuild failed with exit code $LASTEXITCODE"
}

$outputDir = Join-Path $repoRoot "build\msbuild\$Platform\$Configuration"
Write-Host "Build completed successfully. Output directory: $outputDir"
