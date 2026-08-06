# 天猛星例程评审（docs/example_reviews）

对 `examples_and_documents/立创·天猛星MSPM0G3507开发板【模块移植代码】` 中**与本项目阶段直接相关**的例程逐份评审。

> **TI 文件规则（统一措辞）**：`source/ti/driverlib/`＝供应商源码，**不修改**；`ti_msp_dl_config.c/h`＝SysConfig 生成产物，**不手工编辑**（改 `.syscfg` 后重新生成）；`DL_*`＝API 名称，不是文件。本项目适配写在自有 `bsp/` / `drivers/` 层。评审只做借鉴分析，不产生对官方库的任何修改。

## 证据分类（v7.2 起，各评审必须区分）

| 分类 | 含义 |
|---|---|
| `SOURCE_FACT` | 来自源码 / `.syscfg` 的可直接引用事实 |
| `CALCULATED` | 计算推导，须标 `VERIFIED / UNVERIFIED` |
| `INFERRED` | 推断，不得当事实（如模块 PCB 接法） |
| `HARDWARE_CONFIRMATION_REQUIRED` | 须查原理图/万用表/示波器确认 |
| `TARGET_PROJECT_DECISION` | 本项目的设计决策（非参考例程事实） |

## 已评审例程（v7.2 纠偏后）

| 例程 | 对应阶段 | 评审文档 | 结论摘要（纠偏后） |
|---|---|---|---|
| TB6612电机驱动模块 | P4 电机 | [tb6612.md](tb6612.md) | 借鉴"方向 GPIO + PWM"与生成宏；**弃** TIMA1/定时器自启动/Stop=BRAKE/无 STBY/无换向死区；**STBY 模块 PCB 接法未核实（非悬空/上拉）**；80MHz 时钟树不得用于 P1/P2 |
| N20直流减速电机-带霍尔编码器 | P5 编码器 | [n20-hall-encoder.md](n20-hall-encoder.md) | **参考例程是 X2（A、B 两相均 RISE 中断，multiplier=2），非 X1**；弃 `ABS(pwma)` bug/方向符号硬编码/逐周期清零；本项目目标 X1（仅 A 相单沿） |
| SG90舵机 | P7 舵机 | [sg90.md](sg90.md) | **频率 `CALCULATION_UNVERIFIED`（原 40Hz 结论无效，忽略 HFXT/SYSPLL 时钟树）**；弃 ~100Hz 配置/无渐变/无 enable-disable；P7 须重生成 50Hz |
| 0.96寸IIC单色屏（SSD1306） | P8 OLED | [oled-ssd1306-iic.md](oled-ssd1306-iic.md) | **非"一次性大块传输"：约 1056 次独立两字节事务**；`IIC_delay` 为死代码；弃 0x40 多字节/DrawLine 变量错/无超时；仅借鉴 SSD1306 命令/字库 |
| MPU6050六轴传感器 | P9 IMU | [mpu6050.md](mpu6050.md) | 借鉴寄存器初始化序列；**弃**软件 I2C/DMP 栈/无限重试/`(fsr<<3)` 地址错/`num==0` 下溢；**`License.txt` 缺失（`LICENSE_MISSING`），禁止复制 InvenSense 代码** |

## 跨例程共性问题（对本项目 = 全部不照搬）

1. **阻塞 UART 打印**：`uart0_sendChar` 用 `while(DL_UART_isBusy(...))` 阻塞；`lc_printf/LOG_D` 栈上 512B + `vsnprintf` + 阻塞发送。
2. **忙等延时**：`delay_ms/us` 用 `delay_cycles(CPUCLK_FREQ/1000*ms)` 忙等。
3. **PWM 定时器自动启动**：`timerStartTimer = true`。
4. **无安全状态机 / 无 STBY 控制 / 无换向死区**。
5. **zip 携带 `Debug/` 构建产物 + `.clangd` 缓存**（已 gitignore 排除）。
6. **SDK 版本旧**：例程 `@product "mspm0_sdk@2.02.00.05"`、SysConfig **1.21.0**；本项目 SDK 2.10.00.04、SysConfig **1.27.1+4634**（见 `docs/TOOLCHAIN_LOCK.md`）。旧版生成的时钟树语法不能直接搬。
7. **非标准类型**：`u8/u16/u32` 宏。

## 借鉴流程（供 agent/开发者）

1. 读对应评审文档 → 确认 `SOURCE_FACT` 可取部分（协议/寄存器正确性）。
2. 对照 `empty.syscfg` 查天猛星引脚接线（**仅证明该引脚在该参考配置中可解**；本项目以 `control.syscfg` + P1A 结果为准）。
3. **一律按 `PLAN.md` 分层与接口在 `firmware/` 重写**，不复制文件/整段代码/引脚。
4. 引用外部代码记录进 `THIRD_PARTY_NOTICES.md`；`LICENSE_MISSING` / `REVIEW_REQUIRED` 的代码**禁止复制**。
