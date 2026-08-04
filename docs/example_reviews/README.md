# 天猛星例程评审（docs/example_reviews）

对 `examples_and_documents/立创·天猛星MSPM0G3507开发板【模块移植代码】` 中**与本项目阶段直接相关**的例程逐份评审，产出"可取之处 / 不足之处 / 借鉴建议"。

> **铁律（贯穿所有评审）**：**TI 官方 MSPM0 基础底层库（DriverLib：`source/ti/driverlib/`、`ti_msp_dl_config.h/c`、`DL_*` API）绝不允许更改**。例程一律通过 SysConfig + 生成宏使用；所有自定义适配放在我们的 `bsp/`/`drivers/` 层。评审只做借鉴分析，不产生对官方库的任何修改。

## 已评审例程

| 例程 | 对应阶段 | 评审文档 | 结论摘要 |
|---|---|---|---|
| TB6612电机驱动模块 | P4 电机 | [tb6612.md](tb6612.md) | 借鉴"方向 GPIO + PWM"分离与生成宏用法；**弃** TIMA1/定时器自启动/Stop=BRAKE/无 STBY/无换向死区 |
| N20直流减速电机-带霍尔编码器 | P5 编码器 | [n20-hall-encoder.md](n20-hall-encoder.md) | 借鉴 X1 解码与快照计数；**弃**右轮 `ABS(pwma)` 真 bug/方向符号硬编码/Stop 设 9999/逐周期清零 |
| SG90舵机 | P7 舵机 | [sg90.md](sg90.md) | 借鉴角度限幅与 float 映射；**弃** ~40Hz 定时器配置/无渐变/无 enable-disable |
| 0.96寸IIC单色屏（SSD1306） | P8 OLED | [oled-ssd1306-iic.md](oled-ssd1306-iic.md) | 借鉴 SSD1306 完整指令/字库；**弃**整帧阻塞刷新/IIC_delay/无超时轮询 |
| MPU6050六轴传感器 | P9 IMU | [mpu6050.md](mpu6050.md) | 借鉴寄存器初始化序列/WHO_AM_I；**弃**软件 I2C/DMP 栈/无限重试 |

## 跨例程共性问题（对本项目 = 全部不照搬）

1. **阻塞 UART 打印**：`uart0_sendChar` 用 `while(DL_UART_isBusy(...))` 阻塞；`lc_printf/LOG_D` 栈上 512B + `vsnprintf` + 阻塞发送 → 违反本项目"非阻塞调试队列、不阻塞控制循环"。
2. **忙等延时**：`delay_ms/us` 用 `delay_cycles(CPUCLK_FREQ/1000*ms)` 忙等 → 本项目用 1ms 节拍 + 非阻塞调度。
3. **PWM 定时器自动启动**：`empty.syscfg` 中 `timerStartTimer = true` → 上电即输出，违反本项目"PWM 初始化后不得自动启动、执行器默认禁用"。
4. **无安全状态机 / 无 STBY 控制 / 无换向死区**：仅演示用，未处理安全。
5. **zip 携带 `Debug/` 构建产物 + `.clangd` 缓存**：仓库维护差。
6. **SDK 版本旧**：`@product "mspm0_sdk@2.02.00.05"`、SysConfig 1.21.0（本项目 2.10.00.04 / 1.26.2）；旧版生成的时钟树语法不能直接搬。
7. **非标准类型**：`u8/u16/u32` 宏；本项目用 stdint。

## 借鉴流程（供 agent/开发者）

1. 读对应评审文档 → 确认**可取**部分（协议/寄存器正确性）。
2. 对照 `empty.syscfg` 查天猛星引脚接线（板级正确性）。
3. **一律按 `PLAN.md` 分层与接口在 `firmware/` 重写**，不复制文件/整段代码/引脚。
4. 引用外部代码记录进 `THIRD_PARTY_NOTICES.md`（保留原版权头）。
