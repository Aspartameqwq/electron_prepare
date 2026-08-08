# scripts/test_host.ps1 — 编译并运行 host 测试（纯算法模块）
# 用法：
#   powershell -ExecutionPolicy Bypass -File scripts/test_host.ps1            # 常规
#   powershell -ExecutionPolicy Bypass -File scripts/test_host.ps1 -Sanitize   # 启用 ASan/UBSan
# 规范见 docs/HOST_TEST.md：clang / C11 / -Wall -Wextra -Werror；
# 编译命令与结果写入日志 logs/tmp/host/<test>.log；任一失败返回非零退出码。
param([switch]$Sanitize)

$ErrorActionPreference = 'Stop'

$CC     = if ($env:CC) { $env:CC } else { 'clang' }
$Common = @('-std=c11', '-Wall', '-Wextra', '-Werror')
if ($Sanitize) {
    $Common += @('-fsanitize=address,undefined', '-fno-omit-frame-pointer')
}
$Inc    = @('-I', 'firmware/middleware', '-I', 'firmware/config')
$Src    = @(
    'firmware/middleware/ring_buffer.c',
    'firmware/middleware/frame_codec.c'
)
$OutDir = 'logs/tmp/host'
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$tests = @(
    @{ name = 'test_ring_buffer'; src = 'tests/host/test_ring_buffer.c' },
    @{ name = 'test_frame_codec'; src = 'tests/host/test_frame_codec.c' }
)

$failed = $false
foreach ($t in $tests) {
    $exe  = Join-Path $OutDir ($t.name + '.exe')
    $log  = Join-Path $OutDir ($t.name + '.log')
    $args = $Common + $Inc + $Src + @($t.src) + @('-o', $exe)

    ("$CC " + ($args -join ' ')) | Out-File -FilePath $log -Encoding utf8   # 记录编译命令

    Write-Host "== compile $($t.name) =="
    & $CC @args 2>&1 | Tee-Object -FilePath $log -Append
    if ($LASTEXITCODE -ne 0) {
        Write-Host "COMPILE FAILED: $($t.name) (log: $log)"
        $failed = $true
        continue
    }

    Write-Host "== run $($t.name) =="
    & $exe 2>&1 | Tee-Object -FilePath $log -Append
    if ($LASTEXITCODE -ne 0) {
        Write-Host "TEST FAILED: $($t.name) (exit $LASTEXITCODE, log: $log)"
        $failed = $true
    }
}

if ($failed) {
    Write-Host 'HOST TESTS: FAILED'
    exit 1
}
Write-Host 'HOST TESTS: ALL PASSED'
exit 0
