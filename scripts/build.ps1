# scripts/build.ps1 - firmware build via CCS headless CLI (machine-checkable gates)
# Usage: powershell -ExecutionPolicy Bypass -File scripts/build.ps1 [-Clean]
# Gates (all must pass to return 0):
#   1) projectCreate/projectBuild exit code = 0
#   2) SysConfig: 0 error; warning count must EXACTLY equal the approved whitelist (HFXT x2, see docs/ERRATA_CHECKLIST.md);
#      any new/unknown SysConfig warning or unparseable summary -> BUILD FAILED
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
# SysConfig summary line: "N error(s), M warning(s)" — unparseable summary = FAIL
$scErr  = if ($logText -match '(\d+)\s+error\(s\)')    { [int]$Matches[1] } else { $null }
$scWarn = if ($logText -match '(\d+)\s+warning\(s\)')  { [int]$Matches[1] } else { $null }
if ($null -eq $scErr -or $null -eq $scWarn) {
    Write-Error "BUILD FAILED: SysConfig summary line not parseable ('N error(s), M warning(s)' missing) - log: $LOG"
}
# Compiler/linker diagnostic lines: anything matching warning:/error: that is NOT an approved
# SysConfig whitelist item counts as a compiler/linker diagnostic (whitelist text goes through
# the SysConfig summary gate below, not the compiler gate).
$knownExempt = @(
    'HFXT\(/ti/clockTree/pinFunction\.js\) peripheral\.hfxInPin: Solution may have changed',
    'HFXT\(/ti/clockTree/pinFunction\.js\) peripheral\.hfxOutPin: Solution may have changed'
)
$compilerWarn = 0; $compilerErr = 0
$syscfgWarnLines = 0
foreach ($line in (Get-Content $LOG)) {
    $isExempt = $false
    foreach ($pat in $knownExempt) { if ($line -match $pat) { $isExempt = $true; break } }
    if ($isExempt) {
        $syscfgWarnLines++          # approved SysConfig hint occurrences (whitelist members)
        continue
    }
    if ($line -match '(^|\s)warning:\s') { $compilerWarn++ }
    if ($line -match '(^|\s)error:\s')   { $compilerErr++ }
}

# SysConfig warnings must EXACTLY match the approved whitelist:
#   - whitelist occurrence count must equal the SysConfig summary warning count
#     (summary==count 时任一未匹配 warning 都会使两者不相等 -> FAIL，无法解析入 summary 的警告不计入)
#   - any non-exempt warning line (compiler/linker/SysConfig new) -> FAIL (compiler/linker must be 0)
$scWarnFailures = @()
if ($syscfgWarnLines -ne $scWarn) {
    $scWarnFailures += "SysConfig warning count mismatch: summary=$scWarn but whitelist-matched lines=$syscfgWarnLines (new/unknown warnings?)"
}
if ($compilerWarn -gt 0) {
    $scWarnFailures += "non-whitelist warning lines present: $compilerWarn"
}

Write-Host "SysConfig : error=$scErr warning=$scWarn (whitelist HFXT x2; occurrences matched: $syscfgWarnLines; see docs/ERRATA_CHECKLIST.md)"
Write-Host "Compiler  : error=$compilerErr warning=$compilerWarn"

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
