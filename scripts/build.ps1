# scripts/build.ps1 — P1+ 固件构建（CCS headless CLI）
# 用法： powershell -ExecutionPolicy Bypass -File scripts/build.ps1 [-Clean]
# 依赖： CCS 20.5.1（ccs-server-cli）、MSPM0 SDK 2.10（product.json）、SysConfig 1.27.1
# 路径来源： scripts/env.local.ps1（gitignored，模板见 env.example.ps1）
param([switch]$Clean)

$ErrorActionPreference = 'Stop'
$Repo = $PSScriptRoot | Split-Path

# 本机环境（CCS_HOME / MSPM0_SDK_ROOT）
$envFile = Join-Path $Repo 'scripts/env.local.ps1'
if (Test-Path $envFile) { . $envFile }
else { Write-Error "缺少 scripts/env.local.ps1（复制 env.example.ps1 并填本机路径）" }

$CCS_CLI = Join-Path $env:CCS_HOME 'ccs/eclipse/ccs-server-cli.bat'
$WS      = Join-Path $Repo 'logs/tmp/ccs_ws'          # headless workspace（logs/tmp 已 gitignore）
$PROJ    = 'firmware_p1_bringup'
$OUT     = Join-Path $Repo 'firmware/Debug/firmware_p1_bringup.out'

# 1) 从 projectspec 创建工程（幂等：已存在则跳过）
& $CCS_CLI -workspace $WS -application projectCreate `
    -ccs.projectSpec (Join-Path $Repo 'firmware/p1_bringup.projectspec') `
    -ccs.location (Join-Path $Repo 'firmware') 2>&1 | Select-Object -Last 2

# 2) 构建
$buildArgs = @('-workspace', $WS, '-application', 'projectBuild', '-ccs.projects', $PROJ)
if ($Clean) { $buildArgs += '-ccs.buildCommand', 'gmake -k -j 32 -B all -r -O' }
& $CCS_CLI @buildArgs 2>&1 | Tee-Object -FilePath (Join-Path $Repo 'logs/tmp/build.log')

# 3) 校验产物
if (Test-Path $OUT) {
    Write-Host "BUILD OK: $OUT"
    exit 0
}
Write-Error "BUILD FAILED（产物不存在），日志 logs/tmp/build.log"
