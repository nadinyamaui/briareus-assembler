$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
./build.ps1
python tests/make_fixtures.py
if ($LASTEXITCODE -ne 0) { throw 'Fixture generation failed' }
$env:INCLUDE = Join-Path $PSScriptRoot 'tools/INCLUDE'
& ./tools/FASM.EXE -d TEST_MODE=1 src/main.asm build/Briareus-test.exe
if ($LASTEXITCODE -ne 0) { throw 'Fixture build failed' }
python tests/integration.py
if ($LASTEXITCODE -ne 0) { throw 'Native integration tests failed' }
python tests/responsive.py
if ($LASTEXITCODE -ne 0) { throw 'Responsive layout tests failed' }
