# 例程评审：N20 直流减速电机-带霍尔编码器（P5 参考）

**source_id**：`lckfb-tmx-n20-encoder`
**来源**：`examples_and_documents/立创·天猛星MSPM0G3507开发板【模块移植代码】/控制类/N20直流减速电机-带霍尔编码器/`
**结构**：`BSP/src/bsp_motor_hallencoder.c` + `BSP/inc/bsp_motor_hallencoder.h`；`empty.c`；`empty.syscfg`
**SDK**：mspm0_sdk@2.02.00.05 / SysConfig 1.21.0（旧）

## 解码方式（v7.2 纠偏：参考例程是 X2，不是 X1）

**证据（SOURCE_FACT，来自 `empty.syscfg`）**：
- 编码器 4 个引脚 `E1A/E1B/E2A/E2B` 均配置 `interruptEn = true`、`polarity = "RISE"`（上升沿中断）；
- `bsp_motor_hallencoder.c` 的 `GROUP1_IRQHandler` 同时处理 A 相和 B 相中断位（`if ... E1A_PIN ... else if ... E1B_PIN`）。

**结论**：每个正交周期 A、B 两相各触发一次 → **decode_multiplier = 2，属 X2**。

**本项目目标方案（TARGET_PROJECT_DECISION）**：仅 A 相上升沿中断、读 B 相判向 → **X1，decode_multiplier = 1**。
> ⚠️ 两者只能借鉴**判向逻辑**（沿进中断 + 读另一相电平），**不得直接复用该例程的每转计数参数**（否则会把计数放大/缩小一倍）。

## 可取之处（借鉴）

1. 判向思路：沿进中断后读另一相电平判方向——可借鉴，但本项目只用 A 相单沿（X1）。
2. 快照计数模式：`Should_Get_Encoder_Count`（ISR 累计）+ `Obtained_Get_Encoder_Count`（定时器周期快照）——避免主循环直接竞争；本项目可借鉴其"快照"思想，但改为单调模计数 `position_mod` + 20ms 取差（不逐周期清零）。
3. 中断清标志：`DL_GPIO_getEnabledInterruptStatus(...)` + `clearInterruptStatus`——标准做法。
4. 定时器中断用 `DL_TIMER_IIDX_ZERO`——与 mspm0-ccs skill 已验证模式一致。

## 不足之处（弃用）

1. **真 bug（SOURCE_FACT）**：`Motor_Set_PWM(int pwma, int pwmb)` 的 pwmb 分支误用 `ABS(pwma)` → 右轮速度跟了左轮。照搬必错。
2. **`else if` 丢失事件（SOURCE_FACT）**：ISR 中 A/B 两相中断位同时置位（毛刺/近同时跳变）时只处理 A 忽略 B，随后 `clearInterruptStatus` 一并清除 → 计数丢失。本项目用单调模计数 + 逐位处理可规避。
3. **逐周期清零竞争（SOURCE_FACT）**：`Should_Get_Encoder_Count = 0` 若与 ISR 竞争会丢计数 → 本项目用 `position_mod` 单调计数，读增量不清零。
4. **方向符号硬编码在驱动（SOURCE_FACT）**：`Encoder_B.Obtained = -Should` 把机械镜像安装写死在驱动 → 本项目用 `ENCODER_*_DIRECTION_SIGN` 配置解耦。
5. **`Motor_Stop()` 设 compare=9999**：语义依赖极性，脆弱；不算"停止"。
6. **双 PWM 通道 H 桥方案**：与本项目"方向 GPIO + 单 PWM"不同，不照搬。
7. **无速度换算**：只给原始计数，未换算 rpm / 每转计数 → 本项目按 `counts_per_output_rev` 等参数标定。

## 借鉴建议（应用到本项目）

- 判向逻辑可参考，但重写为 `drivers/encoder_gpio`：单调模计数、`gpio_irq_dispatch.c` 集中分发（不自定义 IRQHandler）、X1 单沿。
- 编码器接线引脚查该例程 `empty.syscfg`（板级正确性参考），以本项目 `control.syscfg` + P1A 结果为准。

## TI 库铁律

TI DriverLib 与生成文件（`ti_msp_dl_config.*`）**绝不允许修改**。编码器中断由本项目 `bsp/interrupts.c` 读取/清除硬件状态后分发给 `encoder_on_gpio_irq()`。
