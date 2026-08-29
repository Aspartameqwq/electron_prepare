# PINMAP.md — 物理引脚映射（人类可读镜像，无权威）

> **权威性**（PLAN.md §一）：本文件是 `firmware/control.syscfg`（+P1A 预检 DRAFT 行）的人类可读镜像；
> 与 syscfg 冲突时**以 syscfg 为准并更新本表**。逐引脚状态：`DRAFT`（预检可解，待接线冻结）→ `FROZEN`（硬件到位、用户确认接线后冻结；冻结后禁自动换引脚，变更须 ADR）。
>
> 生成来源：P1A 预检 solver（SysConfig 1.27.1）+ 官方例程佐证（见 RESOURCE_MAP 变更记录）。

## 已实施（FROZEN，P1 上板验证）

| 引脚 | 外设 | 信号 | 状态 | 备注 |
|---|---|---|---|---|
| PA5 | HFXT | HFXIN | FROZEN | 40MHz 晶振（板载） |
| PA6 | HFXT | HFXOUT | FROZEN | 40MHz 晶振（板载） |
| PA19 | SWD | SWDIO | FROZEN | XDS110 |
| PA20 | SWD | SWCLK | FROZEN | XDS110 |
| PA10 | UART0 | TX | FROZEN | 板载 CH340 → PC COM13（文本日志） |
| PA11 | UART0 | RX | FROZEN | 板载 CH340 |
| PB22 | GPIO | LED | FROZEN | 板载心跳灯（初始低） |

## DRAFT（P1A 预检可解，待对应阶段接线冻结）

| 引脚 | 外设 | 信号 | 状态 | 备注 |
|---|---|---|---|---|
| PB6 | UART1 | **TX** | DRAFT | K230/HC-04 二进制协议（明确：PB6=TX） |
| PB7 | UART1 | **RX** | DRAFT | （明确：PB7=RX） |
| PA8 | TIMA0 | CCP0 | DRAFT | 左电机 PWM 20kHz（SDK 官方组合） |
| PB9 | TIMA0 | CCP1 | DRAFT | 右电机 PWM 20kHz |
| PA17 | TIMA1 | CCP0 | DRAFT | 舵机 1 PWM 50Hz（嘉立创本板例程组合） |
| PA16 | TIMA1 | CCP1 | DRAFT | 舵机 2 PWM 50Hz |
| PA0 | I2C0 | SDA | DRAFT | OLED(0x3C)+MPU6050(0x68)（嘉立创 OLED 本板组合） |
| PA1 | I2C0 | SCL | DRAFT | |
| PA24 | GPIO | ENC_L_A | DRAFT | 左编码器 A 相（RISE 中断，X1） |
| PA25 | GPIO | ENC_L_B | DRAFT | 左编码器 B 相（输入判向） |
| PA26 | GPIO | ENC_R_A | DRAFT | 右编码器 A 相（RISE 中断） |
| PA27 | GPIO | ENC_R_B | DRAFT | 右编码器 B 相 |
| PA12 | GPIO | TB6612_AIN1 | DRAFT | 左电机方向 1（初始低） |
| PB16 | GPIO | TB6612_AIN2 | DRAFT | 左电机方向 2（初始低） |
| PB15 | GPIO | TB6612_BIN1 | DRAFT | 右电机方向 1（初始低） |
| PB14 | GPIO | TB6612_BIN2 | DRAFT | 右电机方向 2（初始低） |
| PB13 | GPIO | TB6612_STBY | DRAFT | **必须外部下拉 10kΩ**（P4 硬门禁；初始低） |

## 保留 / 禁用

| 引脚 | 状态 | 说明 |
|---|---|---|
| PA2 / PA18 / PA21 / PA23 | **DO_NOT_USE** | 天猛星文档禁用（PA21 同时使 TIMG6 无合法 CCP0 → 舵机改 TIMA1） |
| TIMG8 (PA29/PA30) | 保留 | 可选单轴 HW QEI，默认不启用 |
| TIMG12 | 内部 | 1ms tick，无引脚 |

## 冻结流程

1. 对应硬件（电机/舵机/OLED/MPU6050/编码器/HC-04）到货并实际接线；
2. 用户确认引脚与 DRAFT 一致（或记录实际接线）；
3. 把正式 `control.syscfg` 加入该外设并 assign 相同引脚 → 重建 0 error；
4. 本表对应行改 `FROZEN` 并注明生效 commit。
