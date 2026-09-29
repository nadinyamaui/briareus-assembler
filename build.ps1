param([switch]$Run)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$fasm = Join-Path $PSScriptRoot 'tools/FASM.EXE'
if (!(Test-Path $fasm)) {
    New-Item -ItemType Directory -Force tools | Out-Null
    $zip = Join-Path $PSScriptRoot 'tools/fasm.zip'
    Invoke-WebRequest 'https://flatassembler.net/fasmw17332.zip' -OutFile $zip
    if ((Get-FileHash $zip -Algorithm SHA256).Hash -ne '57CBFFD990908B7526DE48FB642937887BF9298F463C2F4039C8C19534406BAA') { throw 'FASM archive checksum mismatch' }
    Expand-Archive $zip tools -Force
}
New-Item -ItemType Directory -Force build | Out-Null
$env:INCLUDE = Join-Path $PSScriptRoot 'tools/INCLUDE'
& $fasm src/main.asm build/Briareus.exe
if ($LASTEXITCODE -ne 0) { throw 'Assembly failed' }
& $fasm tests/core.asm build/core-tests.exe
if ($LASTEXITCODE -ne 0) { throw 'Test assembly failed' }
& ./build/core-tests.exe
if ($LASTEXITCODE -ne 0) { throw "Assembly core tests failed: $LASTEXITCODE" }
& $fasm tests/connection.asm build/connection-tests.exe
if ($LASTEXITCODE -ne 0) { throw 'Connection test assembly failed' }
& ./build/connection-tests.exe
if ($LASTEXITCODE -ne 0) { throw "Connection tests failed: $LASTEXITCODE" }
if ($Run) { Start-Process (Join-Path $PSScriptRoot 'build/Briareus.exe') }
