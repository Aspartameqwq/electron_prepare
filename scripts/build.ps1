# scripts/build.ps1 - firmware build via CCS headless CLI (machine-checkable gates)
# Usage: powershell -ExecutionPolicy Bypass -File scripts/build.ps1 [-Clean]
# Gates (all must pass to return 0):
#   1) projectCreate/projectBuild exit code = 0
#   2) SysConfig: 0 error; warning set must be an EXACT match of the approved whitelist
#      (each whitelist pattern must appear exactly once; summary count == whitelist size;
#       any unknown/duplicated/disappeared warning or unparseable summary -> BUILD FAILED)
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
# Workspace marker: unique per run. The CCS launcher derives the real Eclipse workspace from this
# path (AppData\Local\Texas Instruments\CCS\.ccs-server\workspaces\<md5>); a FIXED marker caches a
# stale project import so later projectCreate reports "already exists in workspace" and the build
# fails with "Project not open". A per-run unique marker => fresh derived workspace => clean import.
$WS      = Join-Path $Repo ("logs/tmp/ccs_ws_" + [DateTime]::Now.ToString('yyyyMMdd_HHmmss'))
$PROJ    = 'firmware_p1_bringup'
$DBGDIR  = Join-Path $Repo 'firmware/Debug'
$OUT     = Join-Path $DBGDIR 'firmware_p1_bringup.out'
$LOG     = Join-Path $Repo 'logs/tmp/build.log'
New-Item -ItemType Directory -Force -Path (Split-Path $LOG) | Out-Null

# ---- 1) stale-.out false-success guard + stale project-import guard ----
if ($Clean -or (Test-Path $OUT)) {
    if (Test-Path $DBGDIR) { Remove-Item -Recurse -Force $DBGDIR }
    Write-Host 'old build artifacts removed (stale .out guard)'
}
# project-import guard: every run uses a UNIQUE workspace (-> fresh derived workspace), so the
# -ccs.location passed to projectCreate must not carry a prior import. A leftover .project/.cproject/
# .ccsproject/.settings makes projectCreate fail with "A file or directory already exists at location".
# These are gitignored and regenerated from p1_bringup.projectspec, so they are always cleared here.
foreach ($pfx in '.project', '.cproject', '.ccsproject', '.settings') {
    $p = Join-Path $Repo "firmware/$pfx"
    if (Test-Path $p) { Remove-Item -Recurse -Force $p }
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
# SysConfig summary line: "N error(s), M warning(s)" — unparseable summary = FAIL
$scErr  = if ($logText -match '(\d+)\s+error\(s\)')    { [int]$Matches[1] } else { $null }
$scWarn = if ($logText -match '(\d+)\s+warning\(s\)')  { [int]$Matches[1] } else { $null }
if ($null -eq $scErr -or $null -eq $scWarn) {
    Write-Error "BUILD FAILED: SysConfig summary line not parseable ('N error(s), M warning(s)' missing) - log: $LOG"
}
# Compiler/linker diagnostic lines: anything matching warning:/error: that is NOT an approved
# SysConfig whitelist item counts as a compiler/linker diagnostic (whitelist text goes through
# the SysConfig summary gate below, not the compiler gate).
# EXACT-SET gate: every whitelist pattern must appear EXACTLY once; the SysConfig summary warning
# count must equal the whitelist size; any non-whitelist warning line => FAIL. A disappearing
# whitelist warning ALSO fails (forces manual re-review, never silent auto-pass).
$whitelist = @(
    @{ name = 'HFXT hfxInPin';
       pat  = 'HFXT\(/ti/clockTree/pinFunction\.js\) peripheral\.hfxInPin: Solution may have changed' },
    @{ name = 'HFXT hfxOutPin';
       pat  = 'HFXT\(/ti/clockTree/pinFunction\.js\) peripheral\.hfxOutPin: Solution may have changed' }
)
$patCounts = @{}
foreach ($w in $whitelist) { $patCounts[$w.name] = 0 }
$unknownWarn = 0; $compilerErr = 0
foreach ($line in (Get-Content $LOG)) {
    $whitelisted = $false
    foreach ($w in $whitelist) {
        if ($line -match $w.pat) { $patCounts[$w.name]++; $whitelisted = $true; break }
    }
    if ($whitelisted) { continue }
    if ($line -match '(^|\s)warning:\s') { $unknownWarn++ }
    if ($line -match '(^|\s)error:\s')   { $compilerErr++ }
}

$scWarnFailures = @()
if ($scWarn -ne $whitelist.Count) {
    $scWarnFailures += "SysConfig summary warnings=$scWarn != whitelist size=$($whitelist.Count) (warning set changed or a whitelist item disappeared?)"
}
foreach ($w in $whitelist) {
    if ($patCounts[$w.name] -ne 1) {
        $scWarnFailures += "whitelist '$($w.name)' occurrences=$($patCounts[$w.name]) (must be exactly 1; duplicated or missing)"
    }
}
if ($unknownWarn -gt 0) {
    $scWarnFailures += "non-whitelist warning lines present: $unknownWarn"
}

Write-Host "SysConfig : error=$scErr warning=$scWarn (exact-set whitelist: $((($whitelist | ForEach-Object { $_.name + '=' + $patCounts[$_.name] }) -join ', ')); see docs/ERRATA_CHECKLIST.md)"
Write-Host "Compiler  : error=$compilerErr warning=$unknownWarn"

$fail = @()
if ($scErr -ne 0)        { $fail += "SysConfig error=$scErr" }
if ($scWarnFailures.Count -gt 0) { $fail += $scWarnFailures }
if ($compilerErr -ne 0)  { $fail += "compiler error=$compilerErr" }
if (-not (Test-Path $OUT)) { $fail += "artifact missing: $OUT" }

if ($fail.Count -gt 0) {
    Write-Error ("BUILD FAILED: " + ($fail -join '; ') + " - log: $LOG")
}
Write-Host "BUILD OK (gates: exit codes / SysConfig error=0 / warnings exact-whitelist / compiler+linker warning=0 / fresh .out): $OUT"
exit 0
