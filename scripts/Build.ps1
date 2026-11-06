# SPDX-License-Identifier: GPL-2.0-or-later
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$QtRoot,
    [Parameter(Mandatory=$true)][string]$OpenRGBSource,
    [switch]$RunChecks
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$QtRoot = (Resolve-Path -LiteralPath $QtRoot).Path
$OpenRGBSource = (Resolve-Path -LiteralPath $OpenRGBSource).Path
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'Run from an x64 Native Tools Command Prompt for VS 2022 (start PowerShell inside it).'
}
$buildDir = Join-Path $repoRoot 'build'
$distDir = Join-Path $repoRoot 'dist'
$checks = if ($RunChecks) { 'ON' } else { 'OFF' }
& cmake -S $repoRoot -B $buildDir -G 'NMake Makefiles' '-DCMAKE_BUILD_TYPE=Release' "-DCMAKE_PREFIX_PATH=$QtRoot" "-DOPENRGB_SOURCE_DIR=$OpenRGBSource" "-DASUS_LIGHTING_BUILD_CHECKS=$checks"
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
& cmake --build $buildDir
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
$dll = Join-Path $buildDir 'plugins\OpenRGBASUSLighting.dll'
if ($RunChecks) {
    $previousPath = $env:PATH
    $previousPluginPath = $env:QT_PLUGIN_PATH
    try {
        $env:PATH = (Join-Path $QtRoot 'bin') + ';' + $env:PATH
        $env:QT_PLUGIN_PATH = Join-Path $QtRoot 'plugins'
        & (Join-Path $buildDir 'ASUSLightingChecks.exe') $dll
        if ($LASTEXITCODE -ne 0) { throw 'ASUS plugin checks failed.' }
        & (Join-Path $buildDir 'checks\VoidChecks.exe') (Join-Path $buildDir 'plugins\OpenRGBCorsairVoidLighting.dll')
        if ($LASTEXITCODE -ne 0) { throw 'VOID plugin checks failed.' }
    } finally {
        $env:PATH = $previousPath
        $env:QT_PLUGIN_PATH = $previousPluginPath
    }
}
New-Item -ItemType Directory -Path $distDir -Force | Out-Null
Copy-Item -LiteralPath $dll -Destination (Join-Path $distDir 'OpenRGBASUSLighting.dll') -Force
$hash = (Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash.ToLowerInvariant()
"$hash  OpenRGBASUSLighting.dll" | Set-Content -LiteralPath (Join-Path $distDir 'SHA256SUMS.txt') -Encoding ascii
$voidDll = Join-Path $buildDir 'plugins\OpenRGBCorsairVoidLighting.dll'
Copy-Item -LiteralPath $voidDll -Destination (Join-Path $distDir 'OpenRGBCorsairVoidLighting.dll') -Force
$voidHash = (Get-FileHash -LiteralPath $voidDll -Algorithm SHA256).Hash.ToLowerInvariant()
"$voidHash  OpenRGBCorsairVoidLighting.dll" | Add-Content -LiteralPath (Join-Path $distDir 'SHA256SUMS.txt') -Encoding ascii
Write-Host "Both independent plugins are ready: $distDir"
