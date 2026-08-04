# STATUS.md — 阶段结果与模块验证状态

规则（见 PLAN.md §14）：
- **阶段结果**（phase_result）：`COMPLETED` / `SOFTWARE_READY` / `BLOCKED` / `FAILED`
- **模块验证**（module_verification）：`SOURCE_ONLY` / `HOST_TESTED` / `BOARD_TESTED` / `INTEGRATED`
- 测试结果只对记录的 commit 有效；新 commit 不删旧结果（历史证据）。
- `BOARD_TESTED` 必须有用户上板证据（串口日志/照片/口述/仪器结果）。

## 一、阶段结果

| 阶段 | 结果 | 说明 |
|---|---|---|
| P0 | COMPLETED | 工具链锁定 / 硬件档案 / host 规范 / 勘误清单 / 默认调试器 XDS110 |
| P1 | BLOCKED | 待开发板（最小工程 bringup） |
| P3-SOFTWARE | COMPLETED | 通信核心纯算法 + host 测试全过 |
| P3-BOARD | BLOCKED | 待开发板与 UART1 链路（回环测试） |

## 二、模块验证

| 模块 | 状态 | tested_code_commit | evidence_record_commit | 硬件版本 | 测试日期 | 证据 |
|---|---|---|---|---|---|---|
| ring_buffer | HOST_TESTED | `ed63fcf` | `f0ab9c8` | — | 2026-08-04 | `scripts/test_host.ps1` 全过（10 万随机 push/pop 与模型比对） |
| frame_codec | HOST_TESTED | `ed63fcf` | `f0ab9c8` | — | 2026-08-04 | `scripts/test_host.ps1` 全过（CRC 向量 `0x78DA`、1 万随机往返、10 万字节模糊） |

（后续模块按阶段追加；`BOARD_TESTED` 需用户证据后回填硬件版本与证据。）
