# scripts/test_host.ps1
# 编译并运行 host 测试（纯算法模块）。
# 用法： powershell -ExecutionPolicy Bypass -File scripts/test_host.ps1
# 规范见 docs/HOST_TEST.md：
#   - 编译器 clang，C11，-Wall -Wextra -Werror；
#   - 编译项目真实 C 源码（不用 Python 对照实现）；
#   - 任一测试失败返回非零退出码。
$ErrorActionPreference = 'Stop'

$CC    = if ($env:CC) { $env:CC } else { 'clang' }
$Common = @('-std=c11', '-Wall', '-Wextra', '-Werror')
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
    $exe   = Join-Path $OutDir ($t.name + '.exe')
    $args  = $Common + $Inc + $Src + @($t.src) + @('-o', $exe)

    Write-Host "== compile $($t.name) =="
    & $CC @args 2>&1 | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) {
        Write-Host "COMPILE FAILED: $($t.name)"
        $failed = $true
        continue
    }

    Write-Host "== run $($t.name) =="
    & $exe
    if ($LASTEXITCODE -ne 0) {
        Write-Host "TEST FAILED: $($t.name) (exit $LASTEXITCODE)"
        $failed = $true
    }
}

if ($failed) {
    Write-Host 'HOST TESTS: FAILED'
    exit 1
}
Write-Host 'HOST TESTS: ALL PASSED'
exit 0
