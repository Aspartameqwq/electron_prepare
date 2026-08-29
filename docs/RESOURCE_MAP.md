# RESOURCE_MAP.md — 外设资源规划（P1A 预检后 DRAFT）

> **权威性**（PLAN.md §一）：本表记录"哪个外设实例做什么"；`firmware/control.syscfg` 仍是外设配置唯一机器事实源。
> 与 syscfg 冲突时：**停止，不得自动更换实例**。
> 引脚级细节见 `docs/PINMAP.md`（逐引脚 DRAFT/FROZEN）。
>
> **来源**：P1A 预检（`docs/preflight/pin_preflight.syscfg`，NON_BUILDING_REFERENCE）经 SysConfig 1.27.1 求解，
> **0 error / 7 warning（全部书面豁免：HFXT×2 + TB6612 solver DRAFT 提示×5，见 docs/ERRATA_CHECKLIST.md）**。
> 合法引脚组合依据：TI SDK 官方例程（tima_dead_band=PA8/PB9、trig_stop_restore=PB4/PB1）+ 嘉立创天猛星本板例程（TB6612=TIMA1 PA17/PA16、OLED I2C0=PA0/PA1）。

## 阶段实施状态

| 外设 | 实例 | 用途 | 状态 |
|---|---|---|---|
| SYSCTL/HFXT | SYSPLL+HFXT 40MHz | 80MHz 正式基线 | **已实施**（P1 收口） |
| DEBUGSS | SWD PA19/PA20 | XDS110 调试 | **已实施** |
| GPIO LED | PB22 | 心跳灯 | **已实施** |
| UART0 | PA10/PA11 | CH340 调试日志 | **已实施** |
| TIMG12 | 1ms tick | 时间基准（P2） | DRAFT（预检可分配 ✓） |
| UART1 | PB6(TX)/PB7(RX) | K230/HC-04 协议（P3） | DRAFT（板级固定，PINMAP 注明 TX/RX） |
| TIMA0 | 双电机 PWM 20kHz | P4 | DRAFT（CCP0=PA8, CCP1=PB9） |
| GPIO ×5 | TB6612 方向+STBY | P4 | DRAFT（PA12, PB13-16；STBY 须外部下拉） |
| GPIO ×4 | 编码器 A/B | P5 | DRAFT（L:PA24/25, R:PA26/27；A 相 RISE 中断 X1） |
| TIMA1 | 双舵机 PWM 50Hz | P7 | DRAFT（CCP0=PA17, CCP1=PA16） |
| I2C0 | OLED+MPU6050 | P8/P9 | DRAFT（SDA=PA0, SCL=PA1） |

**规划变更记录（P1A）**：双舵机定时器由原计划的 **TIMG6 改为 TIMA1**——预检证实 TIMG6 在 LQFP-64 上可引出的 CCP0 引脚仅 PA21（天猛星 DO_NOT_USE）/PA0/PA8（TIMA0 专属），无合法解；TIMA1 有嘉立创本板例程验证组合（PA17/PA16）。此为实例变更决策：依据预检数据 + 官方例程证据，**非静默更换**。

## 约束（不变）

- FIXED_BOARD_FUNCTION：PA5/PA6(HFXT)、PA19/PA20(SWD)、PA10/PA11(UART0/CH340)、PB22(LED)
- DO_NOT_USE（天猛星文档）：PA02、PA18、PA21、PA23 —— 本表所有 DRAFT 均已避开
- TIMG8 保留为可选单轴 HW QEI（默认不启用）；TIMG7 备用
- PWM 定时器初始化后禁自动启动（timerStartTimer=false，P4/P7 门禁）
