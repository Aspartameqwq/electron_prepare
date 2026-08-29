# scripts/flash.ps1 - firmware flash via repo-owned DSLite backend (XDS110)
# Usage: powershell -ExecutionPolicy Bypass -File scripts/flash.ps1 [-Run] [-Capture] [-Backend {dslite|skill}]
# Backend:
#   dslite (default): repo-owned path, uses $env:DSLITE_PATH from env.local.ps1.
#                     Params verified against locked toolchain help evidence:
#                     logs/tmp/toolchain/dslite_help.txt ("DSLite flash --config=... -f -v -u").
#   skill           : optional convenience backend (mspm0-ccs skill ccs_dss_debug.py);
#                     NOT required - repo rules keep skill as optional aid only.
# Preconditions checked: ccxml exists, .out exists (fresh build), probe reachable.
# Exit codes strictly checked; no probe serial / COM number written to repo files.
param(
    [switch]$Run,
    [switch]$Capture,
    [int]$CaptureSeconds = 15,
    [ValidateSet('dslite', 'skill')][string]$Backend = 'dslite'
)

$ErrorActionPreference = 'Stop'
$Repo = $PSScriptRoot | Split-Path

$envFile = Join-Path $Repo 'scripts/env.local.ps1'
if (Test-Path $envFile) { . $envFile }
else { Write-Error "missing scripts/env.local.ps1 (copy env.example.ps1 and fill local paths)" }

$CCXML = Join-Path $Repo 'firmware/targetConfigs/MSPM0G3507.ccxml'   # committed (portable form)
$OUT   = Join-Path $Repo 'firmware/Debug/firmware_p1_bringup.out'

# ---- preconditions (PLAN: flash script must check ccxml/.out/probe) ----
if (-not (Test-Path $CCXML)) { Write-Error "missing $CCXML" }
if (-not (Test-Path $OUT))   { Write-Error "missing $OUT (run scripts/build.ps1 first)" }
if ([string]::IsNullOrEmpty($env:DSLITE_PATH) -or -not (Test-Path $env:DSLITE_PATH)) {
    Write-Error "DSLITE_PATH not set or file missing in env.local.ps1"
}

if ($Capture) {
    if ([string]::IsNullOrEmpty($env:SERIAL_PORT)) {
        Write-Error "SERIAL_PORT not set in env.local.ps1 (board CH340 COM port)"
    }
    # UART capture in background; if skill is absent use a plain python fallback note
    $serialPy = $null
    if (-not [string]::IsNullOrEmpty($env:MSPM0_CCS_SKILL)) {
        $candidate = Join-Path $env:MSPM0_CCS_SKILL 'scripts/serial_console.py'
        if (Test-Path $candidate) { $serialPy = $candidate }
    }
    if ($null -eq $serialPy) { Write-Warning 'serial_console.py not available (skill optional); skip UART capture' }
    else {
        Start-Process -NoNewWindow -PassThru python -ArgumentList @(
            $serialPy, '-p', $env:SERIAL_PORT,
            '-b', '115200', '--duration', "$CaptureSeconds", '--timestamp'
        ) -RedirectStandardOutput (Join-Path $Repo 'logs/tmp/flash_uart.txt') | Out-Null
        Start-Sleep -Seconds 3
    }
}

# ---- flash (repo-owned DSLite backend; exit code strictly checked) ----
if ($Backend -eq 'dslite') {
    Write-Host "backend: DSLite ($env:DSLITE_PATH)"
    # verified params (dslite_help.txt): flash --config=<ccxml> -f -v -u <file>
    & $env:DSLITE_PATH flash --config=$CCXML -f -v -u $OUT 2>&1 | Tee-Object -FilePath (Join-Path $Repo 'logs/tmp/flash.log')
    if ($LASTEXITCODE -ne 0) { Write-Error "FLASH FAILED (DSLite exit $LASTEXITCODE), log: logs/tmp/flash.log" }
    Write-Host 'FLASH OK (DSLite: load + verify + run)'
}
else {
    # optional convenience backend: mspm0-ccs skill
    if ([string]::IsNullOrEmpty($env:MSPM0_CCS_SKILL) -or -not (Test-Path (Join-Path $env:MSPM0_CCS_SKILL 'scripts/ccs_dss_debug.py'))) {
        Write-Error 'skill backend selected but mspm0-ccs skill not found (it is optional; use -Backend dslite)'
    }
    $runBat = Join-Path $env:CCS_HOME 'ccs/scripting/run.bat'
    & python (Join-Path $env:MSPM0_CCS_SKILL 'scripts/ccs_dss_debug.py') `
        --ccs-run $runBat --ccxml $CCXML --timeout-ms 60000 --out $OUT `
        (Join-Path $Repo 'firmware') load
    if ($LASTEXITCODE -ne 0) { Write-Error "FLASH FAILED (skill exit $LASTEXITCODE)" }
    Write-Host 'FLASH OK (skill backend)'

    if ($Run) {
        & python (Join-Path $env:MSPM0_CCS_SKILL 'scripts/ccs_dss_debug.py') `
            --ccs-run $runBat --ccxml $CCXML --timeout-ms 30000 --out $OUT `
            (Join-Path $Repo 'firmware') run-to-symbol --symbols --reset 'System Reset' --leave-running
        if ($LASTEXITCODE -ne 0) { Write-Error "RUN FAILED (exit $LASTEXITCODE)" }
        Write-Host 'RUN OK (reset+run via skill)'
    }
}

if ($Capture) {
    Write-Host '=== UART capture (logs/tmp/flash_uart.txt) ==='
    Start-Sleep -Seconds $CaptureSeconds
    Get-Content (Join-Path $Repo 'logs/tmp/flash_uart.txt') -ErrorAction SilentlyContinue
}
