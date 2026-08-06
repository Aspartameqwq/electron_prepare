# ERRATA_CHECKLIST.md — 芯片勘误检查清单

**状态取值**：`NOT_RELEVANT`(当前配置不涉及) / `HANDLED_BY_SDK`(SDK 已处理) / `APPLICATION_WORKAROUND_REQUIRED`(需本项目额外处理) / `VERIFIED`(已核实)。
**规则**：只检查当前阶段实际用到的功能；SDK 已处理的勘误不得重复实现第二套 workaround；优先对照**本地 SDK 2.10** 示例，不凭记忆重写。依据文档：TI MSPM0 勘误表 **SLAZ742G**（Rev. G，2026-07）。

## 检查记录

| 勘误号 | 涉及阶段 | 当前状态 | 检查日期 | 依据 / 备注 |
|---|---|---|---|---|
| I2C_ERR_13 | P8(I2C) | **APPLICATION_WORKAROUND_REQUIRED** | 2026-08-04 | SDK 2.10 i2c controller 示例含官方 workaround（start 后 delay_cycles 再轮询 BUSY），但本项目 `i2c_bus.c` **尚未实现**；须按官方要求主动延时后轮询 BUSY；**P8 代码实现并板端验证后才改 `VERIFIED`** |
| SYSPLL_ERR_01 | E3(80MHz) | NOT_RELEVANT | 2026-08-04 | 未启用 SYSPLL 前不构成阻塞；E3 启用时须做 FCC 校验 |
| UART_ERR_* | P1/P3(UART) | 待查 | — | P1/P3 阶段对照本地 SDK UART 示例核对，记录具体勘误号 |
| GPIO_ERR_* | P1(GPIO/LED) | 待查 | — | P1 阶段核对 |
| FLASH_ERR_* | 发布/烧录 | 待查 | — | 烧录流程核对 |

## 阶段门禁

- 编译 + 链接 warning 必须 0；SysConfig error 必须 0；SysConfig warning 原则上 0，确实无法消除须在本表书面豁免（含 warning 全文/诊断号/原因/官方依据/风险），**禁止写"已知可忽略"**。
- 每个阶段开始时：按"只查当前用到的功能"更新本表。
- v1 明确**不进入** STOP/STANDBY/SHUTDOWN 低功耗模式（避免引入额外恢复问题）。
