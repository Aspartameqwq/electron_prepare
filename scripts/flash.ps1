# scripts/flash.ps1 — 固件烧录（XDS110 + CCS DSS）
# 用法： powershell -ExecutionPolicy Bypass -File scripts/flash.ps1 [-Run] [-Capture]
# 依赖： mspm0-ccs skill 脚本（ccs_dss_debug.py）+ XDS110 + .ccxml（logs/tmp/toolchain/）
# 烧录后可选 -Run（复位运行）与 -Capture（监听 CH340 串口打印启动日志）
param(
    [switch]$Run,
    [switch]$Capture,
    [int]$CaptureSeconds = 15
)

$ErrorActionPreference = 'Stop'
$Repo = $PSScriptRoot | Split-Path

$envFile = Join-Path $Repo 'scripts/env.local.ps1'
if (Test-Path $envFile) { . $envFile }
else { Write-Error "缺少 scripts/env.local.ps1" }

$CCXML = Join-Path $Repo 'firmware/targetConfigs/MSPM0G3507.ccxml'   # 固化于仓库（相对路径形式）
$OUT   = Join-Path $Repo 'firmware/Debug/firmware_p1_bringup.out'
$SKILL = Join-Path $env:MSPM0_CCS_SKILL 'scripts'

# 前置检查（PLAN：烧录脚本必须检查 .ccxml/.out/探针）
if (-not (Test-Path $CCXML)) { Write-Error "缺少 $CCXML" }
if (-not (Test-Path $OUT))   { Write-Error "缺少 $OUT（先运行 build.ps1）" }

if ($Capture) {
    # 串口监听放后台（CH340 串口号在 env.local.ps1 的 SERIAL_PORT）
    $cap = Start-Process -NoNewWindow -PassThru python -ArgumentList @(
        (Join-Path $SKILL 'serial_console.py'), '-p', $env:SERIAL_PORT,
        '-b', '115200', '--duration', "$CaptureSeconds", '--timestamp'
    ) -RedirectStandardOutput (Join-Path $Repo 'logs/tmp/flash_uart.txt')
    Start-Sleep -Seconds 3
}

# 烧录（load = program flash）
& python (Join-Path $SKILL 'ccs_dss_debug.py') `
    --ccs-run (Join-Path $env:CCS_HOME 'ccs/scripting/run.bat') `
    --ccxml $CCXML --timeout-ms 60000 --out $OUT `
    (Join-Path $Repo 'firmware') load
if ($LASTEXITCODE -ne 0) { Write-Error "FLASH FAILED（退出码 $LASTEXITCODE）" }
Write-Host "FLASH OK"

if ($Run -or $Capture) {
    # 系统复位 + 继续运行（banner 会重新打印，被 -Capture 捕获）
    & python (Join-Path $SKILL 'ccs_dss_debug.py') `
        --ccs-run (Join-Path $env:CCS_HOME 'ccs/scripting/run.bat') `
        --ccxml $CCXML --timeout-ms 30000 --out $OUT `
        (Join-Path $Repo 'firmware') run-to-symbol --symbols --reset 'System Reset' --leave-running
    if ($LASTEXITCODE -ne 0) { Write-Error "RUN FAILED（退出码 $LASTEXITCODE）" }
    Write-Host "RUN OK（复位运行）"
}

if ($Capture) {
    Wait-Process -Id $cap.Id -Timeout ($CaptureSeconds + 5) -ErrorAction SilentlyContinue
    Write-Host "=== UART 捕获（logs/tmp/flash_uart.txt）==="
    Get-Content (Join-Path $Repo 'logs/tmp/flash_uart.txt') -ErrorAction SilentlyContinue
}
