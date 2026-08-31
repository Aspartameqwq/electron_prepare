# ERRATA_CHECKLIST.md — 芯片勘误检查清单

**状态取值**：`NOT_RELEVANT`(当前配置不涉及) / `HANDLED_BY_SDK`(SDK 已处理) / `APPLICATION_WORKAROUND_REQUIRED`(需本项目额外处理) / `VERIFIED`(已核实)。
**规则**：只检查当前阶段实际用到的功能；SDK 已处理的勘误不得重复实现第二套 workaround；优先对照**本地 SDK 2.10** 示例，不凭记忆重写。
**依据文档**：TI MSPM0 勘误表 **SLAZ742H**（JULY 2023 – REVISED AUGUST 2026，41 页全文本已存档 `logs/tmp/toolchain/slaz742g.txt`，gitignored；官方 URL：ti.com/lit/pdf/slaz742）。
**设备头宏判定**：设备专属勘误由 `source/ti/devices/msp/m0p/mspm0g350x.h` 的 `__*_ERR_*__` 宏控制 SDK 行为——G3507 头文件**未定义任何**此类宏（实证：`grep "__.*_ERR_.*__" mspm0g350x.h` 为空）。SDK 对 SYSPLL_ERR_01 等按模块+设备影响函数另行处理（见下）。

## 检查记录（P1 80MHz 基线全量筛查，2026-08-29 复核）

> 本节只列 P1 当前真实使用的功能：**SYSPLL/HFXT 时钟路径、Flash（80MHz 取指）、UART0、GPIO、SWD/debug**。
> "待查" 不是合法状态；不使用的外设一律 `NOT_RELEVANT` 并注明理由，P 阶段将其纳入时再按 SLAZ742H 重新核对。

### P1 使用功能

| 勘误号 | 涉及阶段 | 当前状态 | 检查日期 | 依据 / 备注 |
|---|---|---|---|---|
| **SYSPLL_ERR_01** | P1 | **HANDLED_BY_SDK** | 2026-08-29 | SLAZ742H：SYSPLL 使能时可能锁错频，官方 workaround＝用 FCC 监测 SYSPLL 频率，检测到错误则 disable/re-enable SYSPLL。**本地证据**：① `source/ti/driverlib/.meta/sysctl/SYSCTLMSPM0Clocks.js`（SysConfig SYSCTL 时钟配置）`enableWorkaround_SYSPLL_ERR_01` **默认 true**（displayName "Validate SYSPLL Frequency Lock"）；② `source/ti/driverlib/.meta/Common.js` `isdeviceAffected_SYSPLL_ERR_01()` 对所有 MSPM0 返回 true；③ 生成代码 `firmware/Debug/syscfg/ti_msp_dl_config.c` L108-199：`SYSCFG_DL_SYSCTL_SYSPLL_init()`（FCC 测 SYSPLLCLK2X 与 HFCLK 相对 LFCLK 计数 → 比例界检查 `FCC_EXPECTED_RATIO=2000`（=80/40）±0.3%）+ `SYSCFG_DL_SYSCTL_init()` 内 `[SYSPLL_ERR_01]` 注释的失败 toggle 重锁循环。本项目未覆盖该 WEAK 函数、未改生成文件 → **运行时佐证：P1 clean build 后 80MHz System Reset ×3 均正常启动**；升 `VERIFIED` 待用户 POR 冷启动×3 后由所有者确认 |
| **FLASH_ERR_01** | P1 | **该编号不存在** | 2026-08-29 | SLAZ742H 全文 FLASH 系列实测为 **02/04/05/06/08**，无 `FLASH_ERR_01`（早期记录的编号有误，现澄清）。用户关注的"80MHz 升频前 flash 处理"由下列各行覆盖：等待状态在升频前置位（HANDLED_BY_SDK）、FLASH_ERR_05（Factory Trim 区假读，可忽略）、≥32MHz 状态位清理（SDK info） |
| **FLASH 等待状态（80MHz）** | P1 | **HANDLED_BY_SDK** | 2026-08-29 | SysConfig 生成 `DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2)` 于 `SYSCFG_DL_SYSCTL_init()` **第一处、在 SYSPLL 使能/MCLK 切 80MHz 之前**（`ti_msp_dl_config.c` L172），满足 80MHz 所需 2 wait states（`dl_sysctl_mspm0g1x0x_g3x0x.h` `DL_SYSCTL_FLASH_WAIT_STATE` 枚举）；boot 期仍处 SYSOSC 32MHz 安全区间。P1 不执行 flash 写/擦，无需其他处理 |
| **≥32MHz flash 状态位（非勘误，SysConfig info）** | P1 | **应用不触发** | 2026-08-29 | SysConfig 对 ≥32MHz 配置输出官方提示："For best practices when the CPUCLK is running at 32MHz and above, clear the flash status bit using DL_FlashCTL_executeClearStatus() before executing any flash operation. Otherwise there may be false positives."（见 `logs/tmp/build.log`）。P1 无 flash 编程/擦除操作，不触发；P4 及以后写/擦 flash 前按此执行 |
| FLASH_ERR_02 | P1 | **NOT_RELEVANT** | 2026-08-29 | 仅当 NONMAIN 配 `DEBUGACCESS=0x5566`（debug disable）才涉及；本项目不改 NONMAIN（flash.ps1 亦注明不得默认擦除/修改 NONMAIN） |
| FLASH_ERR_04 | P1 | **NOT_RELEVANT** | 2026-08-29 | 仅 flash ECC 错误地址报告（DEDERRADDR）错误场景；P1 不实现 flash 错误处理 |
| FLASH_ERR_05 | P1 | **NOT_RELEVANT** | 2026-08-29 | `DEDERRADDR` 复位值可为 `0x00C4013C`（位置在 Factory Trim 区，**可安全忽略**）；P1 不读 DEDERRADDR、无 flash 错误处理 |
| FLASH_ERR_06 / FLASH_ERR_08 | P1 | **NOT_RELEVANT** | 2026-08-29 | CPU 与 DMA 并发访问 flash / 非法内存区硬故障；P1 无 DMA、无非法区域访问 |
| **IOMUX_ERR_02** | P1 | **NOT_RELEVANT**（附 P3 门禁） | 2026-08-29 | 80MHz 下 IOMUX **读**时序余量不足（Workaround 1：只写、避免软件读）。P1 **应用零 IOMUX 读**；boot 期 pinmux 均为 SysConfig 生成一次性**写**（HFXIN/HFXOUT/UART-TX/LED 单次 store；UART-RX 经 `DL_GPIO_initPeripheralInputFunctionFeatures`，其末尾 `PINCM \|= wakeup&WUEN_MASK` 为一次 RMW——但 wakeup=DISABLE 时为 `\|=0` 无意义位合并，且属 TI 官方 80MHz 例程同款生成代码）。生成 init 在 MCLK 切 80MHz **之后**执行（SYSCTL→GPIO 顺序），80MHz System Reset ×3 板验期间 UART0 TX/LED 均正常。**P3 门禁**：UART0 RX / UART1 开始主动读 IOMUX 前，按 SLAZ742H Workaround 2（临时降 MCLK≤40MHz）或 Workaround 3（丢弃首读的双读）复核 |
| **UART_ERR_01** | P1/P3 | **NOT_RELEVANT** | 2026-08-29 | "UART start 条件在 STANDBY1 转换时无法检测"——P1 不进 STANDBY1，UART 常开 |
| **UART_ERR_02** | P1/P3 | **NOT_RELEVANT** | 2026-08-29 | EOT 中断在 TXE-only（CTL0.TXE=1,RXE=0）时不触发——P1 UART0 用 `DL_UART_transmitDataBlocking`（等 TXFIFO 非满 + BUSY），不使用 EOT 中断；RX 引脚已启用（RXE=1），即使未来用 EOT 也满足 TXE+RXE 同开。P3 transport 改用 EOT 中断须复查此项 |
| **UART_ERR_04** | P1/P3 | **NOT_RELEVANT** | 2026-08-29 | fast clock request 被禁用时误收——P1 固定 115200，ULPCLK=40MHz（BUSCLK 源），无时钟切换/FCR 开关 |
| **UART_ERR_05** | P1 | **NOT_RELEVANT** | 2026-08-29 | 调试 halt 功能局限；P1 不依赖 debug halt |
| **UART_ERR_06** | P3+ | **NOT_RELEVANT** | 2026-08-29 | 9-bit 模式 RTOUT/Busy/Async 异常；未使用 9-bit |
| **UART_ERR_07** | P3+ | **NOT_RELEVANT** | 2026-08-29 | IDLE LINE 模式 RTOUT 计数；未使用 IDLE 模式 |
| **UART_ERR_08** | P1/P3 | **NOT_RELEVANT** | 2026-08-29 | STAT.BUSY 在 **UART 被禁用后**仍保持高——P1 UART0 **全程 ENABLE**（无禁能/重配置），`transmitDataBlocking` 在模块使能下轮询 TXFIFO+BUSY 语义正确（115200 板验 banner 逐轮正常）。P3/以后若实现 UART 禁能/重配，须按 SLAZ742H workaround 改轮询 TXFIFO + CTL0.ENABLE |
| **UART_ERR_09** | P3+ | **NOT_RELEVANT** | 2026-08-29 | 地址匹配中断在慢速读前未及时置位；未使用 ADDR_MATCH |
| **UART_ERR_10** | — | **NOT_RELEVANT** | 2026-08-29 | 仅 IrDA 模式 BUSY 置位延迟；未使用 IrDA |
| **UART_ERR_11** | P3+ | **NOT_RELEVANT** | 2026-08-29 | STOP bit 期间 RX 超时提前起算；P1 不使用 RTOUT |
| GPIO_ERR_01 | P1 | **NOT_RELEVANT** | 2026-08-29 | 仅涉 STANDBY 唤醒边沿丢失；v1 明确不进 STOP/STANDBY/SHUTDOWN |
| GPIO_ERR_03 | P1 | **NOT_RELEVANT** | 2026-08-29 | 调试器读 GPIO EVENT0 IIDX 会清中断——P1 无 GPIO 中断 |
| GPIO_ERR_04 | P1 | **NOT_RELEVANT** | 2026-08-29 | 全局 fastwake 配 DIN 影响；P1 未配 fastwake，PB22 为输出驱动 LED |
| **GPIO_ERR_06** | P1 | **NOT_RELEVANT** | 2026-08-29 | DMA mask 影响 CPU 写 GPIO DOUT/DOUTSET/DOUTCLR——**G3507 设备头未定义 `__GPIO_ERR_06__`**（G5xx/L111x 专属，dl_gpio.h 按宏分支）；且 P1 无 DMA |
| CLK_ERR_01 | P1 | **NOT_RELEVANT** | 2026-08-29 | 仅 4MHz HFXT 作 MCLK 时 hardfault；本项目 HFXT=40MHz，且 syscfg 开启 HFCLK 故障监测（失效自动切 SYSOSC） |
| FCC_ERR_01 | P1 | **NOT_RELEVANT** | 2026-08-29 | FCC 在 **BUSCLK 源为 LFCLK** 时行为异常；本项目 BUSCLK=ULPCLK（MCLK/2=40MHz）；LFCLK 仅作 SYSPLL_ERR_01 workaround 的 FCC **触发源**（官方用法） |
| SYSCTL_ERR_02/03/04 | P1 | **NOT_RELEVANT** | 2026-08-29 | SYSSTATUS.FLASHSEC / DEDERRADDR 复位语义；P1 不读这些状态 |
| SYSCTL_ERR_05 | P1 | **NOT_RELEVANT** | 2026-08-29 | 退出 SHUTDOWN 后 LFCLK 不可用；无 SHUTDOWN |
| SYSCTL_ERR_06 (CLK_OUT 毛刺) | P1 | **NOT_RELEVANT** | 2026-08-29 | 外部 CLK_OUT 禁用时毛刺；未用 CLK_OUT |
| SYSOSC_ERR_01/02/04/05/06 系 | P1 | **NOT_RELEVANT** | 2026-08-29 | 均涉 FCL/STOP/LPM/异步/MFCLK 场景；P1 仅 RUN 模式，SYSOSC 仅 boot 基频 + HFCLK 故障监测备用，不启用 FCL、不进 LPM |
| CPU_ERR_01/02/03/04 | P1 | **NOT_RELEVANT** | 2026-08-29 | cache/prefetch 仅在"Main↔NONMAIN/Factory 区切换访问、LPM 过渡、hard-fault 恢复"场景有风险；P1 全程主 flash 取指、RUN 模式、无 hard-fault 恢复代码。P2 起引入定时器等前若启用 ICACHE 相关功能按 SLAZ742H 各条核对 |
| SRAM_ERR_02 | P1 | **NOT_RELEVANT** | 2026-08-29 | SRAM 错误地址寄存器 msb 缺失；P1 不实现 SRAM 错误处理 |
| BSL_ERR_01 | P1 | **NOT_RELEVANT** | 2026-08-29 | 应用软件调用 BSL 失败条件；未用 BSL |
| RST_ERR_01/02 | P1 | **NOT_RELEVANT** | 2026-08-29 | NRST 释放沿与 LFOSCGOOD/初始上电时序竞争（幂等复位时序）；P1 复位原因仅读取记录，无 NRST 时序操作。冷启动×3 用户验收若复位原因与预期不符再按此复查 |

### 不使用功能（预登记，P4/P8/E4 等纳入时按 SLAZ742H 逐条核对）

| 勘误号 | 涉及阶段 | 当前状态 | 检查日期 | 依据 / 备注 |
|---|---|---|---|---|
| TIMER_ERR_01/04/06/07 | P2 起 | **NOT_RELEVANT**（P1 未用定时器） | 2026-08-29 | P2 启用 TIMG12 时按 SLAZ742H 对应条目核对 |
| **I2C_ERR_13** | P8(I2C) | **APPLICATION_WORKAROUND_REQUIRED** | 2026-08-04→2026-08-29 复核 | SLAZ742H 确认：BURSTRUN 置位后约 3 个 I2C 功能时钟 BUSY 才置位，立即轮询可能误判完成（高 CLKDIV/高优化更易发）。官方 workaround＝启动后延时再轮询。SDK 2.10 i2c controller 示例含该处理（`i2c_controller_rw_multibyte_fifo_poll.c`），本项目 `i2c_bus.c` **尚未实现**；P8 实现并板端验证后改 `VERIFIED` |
| WWDT_ERR_01/02 | E4 | NOT_RELEVANT | 2026-08-29 | WWDT 未启用（E4 才启用） |
| ADC/COMP/DAC/DMA/SPI/CRCP/MATHACL/VREF/PMCU/PWREN/RTC 系 | P4/P8/E 各阶段 | NOT_RELEVANT | 2026-08-29 | P1 未使用；对应阶段纳入时按 SLAZ742H 核对 |

## SysConfig warning 白名单（精确匹配，新增即 BUILD FAILED）

- **正式构建**（`scripts/build.ps1` 对 `firmware/control.syscfg`）：error=0；warning **必须精确等于 2 条 HFXT**（`HFXT(/ti/clockTree/pinFunction.js) peripheral.hfxInPin/hfxOutPin: Solution may have changed`），逐行命中白名单，出现任意新增 warning 即 BUILD FAILED。**加大写：禁止"已知可忽略"类豁免。**
- **P1A 预检**（`docs/preflight/pin_preflight.syscfg`，NON_BUILDING，独立运行 SysConfig）：0 error，**7 条 warning 全部书面豁免**＝HFXT×2（同正式构建原因）＋TB6612_DIR 5 个 GPIO 引脚 `Solution may have changed`×5（DRAFT 语义：预检 `$suggestSolution` 未 assign，交由 solver 分配；正式阶段 assign 具体引脚后这类提示消失）。
- 若 SDK/SysConfig 版本变化使 HFXT 提示数量变化，须先更新白名单与本表再验收。

## 阶段门禁

- 编译 + 链接 warning 必须 0；SysConfig error 必须 0；SysConfig warning 白名单**精确匹配**（见上）；新增任意 warning 即 BUILD FAILED，禁止写"已知可忽略"。
- 每个阶段开始时：按"只查当前用到的功能"更新本表（依据 SLAZ742H）。
- v1 明确**不进入** STOP/STANDBY/SHUTDOWN 低功耗模式（避免引入额外恢复问题）。