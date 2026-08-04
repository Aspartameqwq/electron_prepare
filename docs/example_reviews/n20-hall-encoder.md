# 例程评审：N20 直流减速电机-带霍尔编码器（P5 参考）

**来源**：`examples_and_documents/立创·天猛星MSPM0G3507开发板【模块移植代码】/控制类/N20直流减速电机-带霍尔编码器/`
**结构**：`BSP/inc/bsp_motor_hallencoder.h` + `BSP/src/bsp_motor_hallencoder.c`；`empty.c`；`empty.syscfg`
**SDK**：mspm0_sdk@2.02.00.05（旧）

## 可取之处（借鉴）

1. **GPIO 外部中断 X1 解码**：在 `GROUP1_IRQHandler` 中对 A 相沿进中断、读 B 相电平判方向（B 相沿进中断读 A 相），左右两个编码器一个 ISR 处理——与本项目"软件 X1 解码"默认方案一致。
2. **快照计数模式**：`Should_Get_Encoder_Count`（ISR 累计）+ `Obtained_Get_Encoder_Count`（定时器周期快照），主循环读快照——避免 ISR 与主循环竞争。
3. **中断清标志**：`DL_GPIO_getEnabledInterruptStatus(...)` + 末尾 `clearInterruptStatus`——标准做法。
4. **两电机安装相反的符号处理思路**（右轮取反）——意识到"镜像安装需符号修正"这一工程点（但其实现是硬编码，见不足）。
5. **定时器中断用 `DL_TIMER_IIDX_ZERO`**：与 mspm0-ccs skill 已验证模式一致。

## 不足之处（弃用）

1. **真 bug：右轮 PWM 用了 `ABS(pwma)`**：`Motor_Set_PWM(int pwma, int pwmb)` 的 pwmb 分支里 `DL_TimerG_setCaptureCompareValue(..., ABS(pwma), ...)` ——右轮速度跟了左轮。**照搬必出错**。
2. **`Motor_Stop()` 设 compare=9999**：依赖极性使 9999=0% duty，语义脆弱（换极性即失效）；且不算"停止"，更像置满比较值。
3. **方向符号硬编码在驱动**：`Encoder_B.Obtained = -Should` ——把机械安装方向写死在驱动里 → 本项目用 `ENCODER_*_DIRECTION_SIGN` 配置解耦。
4. **逐周期清零计数**：`Should_Get_Encoder_Count = 0` ——若 ISR 与清零竞争会丢计数 → 本项目用单调模计数 `position_mod`，不关中断读增量。
5. **`else if` 漏事件**：若 A/B 两相中断位同时置位（毛刺/近同时跳变），只处理 A 忽略 B → 可能漏计数。
6. **双 PWM 通道 H 桥方案**：每电机两个 PWM 通道（forward/reverse 各一）——与本项目"方向 GPIO + 单 PWM"方案不同，不照搬。
7. **无速度换算**：只给原始计数，未换算 rpm/每转计数（`counts_per_output_rev` 等需自行标定）→ 本项目按 HARDWARE_PROFILE 区分 `encoder_a_cycles_per_motor_rev / decode_multiplier / motor_revolutions_per_output_revolution`。

## 与 PLAN.md 冲突点（必不照搬）

| 本例子 | 本项目要求 |
|---|---|
| 右轮 `ABS(pwma)`（bug） | 左右独立命令，逐项校验 |
| 方向符号硬编码 | `ENCODER_*_DIRECTION_SIGN` 配置 |
| 逐周期清零 | 单调模计数 + 增量读取 |
| 原始计数 | rpm = Δcount×60000/(每转计数×Δt)，含 stale 处理 |
| 无低速分辨率评估 | `N_window ≥ 4` 门禁 + 自适应测速窗口 |

## 借鉴建议（应用到本项目）

- **解码逻辑**：可参考其"沿进中断 + 判另一相电平"的 X1 思想，但重写为 `drivers/encoder_gpio`（单调模计数、经 `gpio_irq_dispatch.c` 集中分发、不自己定义 IRQHandler）。
- **快照模式**：借鉴 Should/Obtained 思想，但改为 `position_mod` 单调计数 + 20ms 任务取差，不清零。
- **标定**：按 `empty.syscfg` 查编码器接线引脚（板级正确性），参数入 `hardware_config.h`。

## TI 库铁律

TI DriverLib 与生成文件（`ti_msp_dl_config.*`）**绝不允许修改**。编码器中断统一由本项目 `bsp/interrupts.c` 读取/清除硬件状态后分发给 `encoder_on_gpio_irq()`。
