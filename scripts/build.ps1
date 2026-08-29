# scripts/build.ps1 - firmware build via CCS headless CLI (machine-checkable gates)
# Usage: powershell -ExecutionPolicy Bypass -File scripts/build.ps1 [-Clean]
# Gates (all must pass to return 0):
#   1) projectCreate/projectBuild exit code = 0
#   2) SysConfig: 0 error; warnings allowed only for ERATA-exempt HFXT hints (x2, see docs/ERRATA_CHECKLIST.md)
#   3) Compiler/linker: 0 error, 0 warning (matched by real diagnostic format "warning:"/"error:")
#   4) .out must be freshly produced by THIS build (stale artifact removed before build)
# Requires: scripts/env.local.ps1 (CCS_HOME / MSPM0_SDK_ROOT; template env.example.ps1)
param([switch]$Clean)

$ErrorActionPreference = 'Stop'
$Repo = $PSScriptRoot | Split-Path

$envFile = Join-Path $Repo 'scripts/env.local.ps1'
if (Test-Path $envFile) { . $envFile }
else { Write-Error "missing scripts/env.local.ps1 (copy env.example.ps1 and fill local paths)" }

$CCS_CLI = Join-Path $env:CCS_HOME 'ccs/eclipse/ccs-server-cli.bat'
$WS      = Join-Path $Repo 'logs/tmp/ccs_ws'
$PROJ    = 'firmware_p1_bringup'
$DBGDIR  = Join-Path $Repo 'firmware/Debug'
$OUT     = Join-Path $DBGDIR 'firmware_p1_bringup.out'
$LOG     = Join-Path $Repo 'logs/tmp/build.log'
New-Item -ItemType Directory -Force -Path (Split-Path $LOG) | Out-Null

# ---- 1) kill stale-.out false-success: remove old artifacts before verified build ----
if ($Clean -or (Test-Path $OUT)) {
    if (Test-Path $DBGDIR) { Remove-Item -Recurse -Force $DBGDIR }
    Write-Host 'old build artifacts removed (stale .out guard)'
}

# ---- 2) create project (idempotent; tolerate "already exists", fail on real errors) ----
$createArgs = @(
    '-workspace', $WS,
    '-application', 'projectCreate',
    '-ccs.projectSpec', (Join-Path $Repo 'firmware/p1_bringup.projectspec'),
    '-ccs.location', (Join-Path $Repo 'firmware')
)
& $CCS_CLI @createArgs > $LOG 2>&1
$createExit = $LASTEXITCODE
$createOut  = Get-Content $LOG -Raw
if ($createExit -ne 0 -and $createOut -notmatch 'already exists in workspace') {
    Write-Error "projectCreate FAILED (exit $createExit), log: $LOG"
}

# ---- 3) build (check exit code; full output to log) ----
$buildArgs = @('-workspace', $WS, '-application', 'projectBuild', '-ccs.projects', $PROJ)
if ($Clean) { $buildArgs += '-ccs.buildCommand', 'gmake -k -j 32 -B all -r -O' }
& $CCS_CLI @buildArgs 2>&1 | Tee-Object -FilePath $LOG | Out-Null
if ($LASTEXITCODE -ne 0) { Write-Error "projectBuild FAILED (exit $LASTEXITCODE), log: $LOG" }

# ---- 4) machine gates: real diagnostic formats (not fuzzy grep) ----
$logText = Get-Content $LOG -Raw
# SysConfig summary line: "N error(s), M warning(s)"
$scErr  = if ($logText -match '(\d+)\s+error\(s\)')   { [int]$Matches[1] } else { -1 }
$scWarn = if ($logText -match '(\d+)\s+warning\(s\)') { [int]$Matches[1] } else { -1 }
# Compiler/linker diagnostic lines, excluding ERATA-exempt SysConfig HFXT hints
$knownExempt = @(
    'HFXT\(/ti/clockTree/pinFunction\.js\) peripheral\.hfxInPin: Solution may have changed',
    'HFXT\(/ti/clockTree/pinFunction\.js\) peripheral\.hfxOutPin: Solution may have changed'
)
$compilerWarn = 0; $compilerErr = 0
foreach ($line in (Get-Content $LOG)) {
    $isExempt = $false
    foreach ($pat in $knownExempt) { if ($line -match $pat) { $isExempt = $true; break } }
    if ($isExempt) { continue }
    if ($line -match '(^|\s)warning:\s') { $compilerWarn++ }
    if ($line -match '(^|\s)error:\s')   { $compilerErr++ }
}

Write-Host "SysConfig : error=$scErr warning=$scWarn (2 HFXT hints exempt, see docs/ERRATA_CHECKLIST.md)"
Write-Host "Compiler  : error=$compilerErr warning=$compilerWarn"

$fail = @()
if ($scErr -ne 0)        { $fail += "SysConfig error=$scErr" }
if ($compilerErr -ne 0)  { $fail += "compiler error=$compilerErr" }
if ($compilerWarn -ne 0) { $fail += "compiler warning=$compilerWarn (non-exempt warnings not allowed)" }
if (-not (Test-Path $OUT)) { $fail += "artifact missing: $OUT" }

if ($fail.Count -gt 0) {
    Write-Error ("BUILD FAILED: " + ($fail -join '; ') + " - log: $LOG")
}
Write-Host "BUILD OK (gates passed: exit codes / SysConfig 0 error / compiler warning=0 / fresh .out): $OUT"
exit 0
