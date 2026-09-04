# HOST_TEST.md — Host 测试规范

用于**纯算法模块**（ring_buffer / frame_codec / pi_controller / attitude 数学）在 PC 上的单元测试。

**工具链确认（2026-08-04）**：本机 clang 19.1.3 可用 → 纯算法模块允许标记 `HOST_TESTED`。

## 工具链（固定）

| 项 | 值 |
|---|---|
| 编译器 | clang（本机 19.1.3；gcc 13.2.0 备用） |
| C 标准 | C11 |
| 参数 | `-std=c11 -Wall -Wextra -Werror` |
| 可选 | AddressSanitizer / UndefinedBehaviorSanitizer |

## 规则

1. `test_host.ps1` 编译**项目真实 C 源码**并执行生成的测试程序；任一测试失败返回**非零退出码**。
2. **不得用 Python 重写同一算法替代 C 测试**。
3. 随机测试使用**固定种子**并在输出中打印种子（可复现）。
4. 编译命令与结果写入测试日志（`logs/tmp/`）。
5. 算法模块**不得 include MSPM0 SDK 头**，否则 host 无法编译。
6. 本机无原生 C 编译器时禁止标记 `HOST_TESTED`（`clang --version` 检查）。

## 适用模块与状态

纯算法层（ring/frame_codec/PI/attitude 数学）发布前最低独立状态 = `HOST_TESTED`（详见 PLAN.md 十四节 VERIFICATION_MATRIX）。

## 执行入口

- `scripts/test_host.ps1`：**已建立**，ring_buffer / frame_codec host 测试已通过（预研资产，`HOST_TESTED`）。注意 `release_gate = NOT_MET`，已知缺口（时间戳回绕、Sanitizer、空指针/超时校验、`*out_len` 清零、目标端并发）见 `docs/STATUS.md`。
- 可选：P3 host 测试稳定后加 `.github/workflows/host-tests.yml`（只跑纯算法，不构建 CCS/不烧录）。

## P2 测试范围（2026-09-04）

- `test_scheduler.c`：保留 ID/零周期/最大周期/空回调/满表；真实优先级和同级 ID 顺序；同轮时间快照；漏跑不补算；取消本轮任务；重入/回调期间修改保护；600000ms 仿真跨 UINT32_MAX。
- `test_scheduler_app.c`：编译真实验收 APP，只替换 BSP 发送和 LED；验证正常 RESULT_PASS、故意漏跑 RESULT_FAIL、结束后停任务与冻结重发。
- 执行 `powershell -ExecutionPolicy Bypass -File scripts/test_host.ps1` 及 `-Sanitize`；两者均须退出 0。逻辑时间仿真不代表真实 TIMG12 精度或板端 10min 通过。
