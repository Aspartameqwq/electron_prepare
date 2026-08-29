# P1 Bringup 验证记录

> 正式轻量验证记录（大日志在 `logs/tmp/`，gitignored；本记录只留脱敏摘要）。
> 测试结果只对记录的 `tested_code_commit` 有效；新代码使旧结果失效时**不删除历史**，只标注适用范围。

## 记录 2：80MHz 基线（当前版本）

| 字段 | 值 |
|---|---|
| phase | P1 |
| tested_code_commit | （本分支收口 commit，见 git log） |
| evidence_record_commit | （同上） |
| firmware configuration | `control.syscfg`：HFXT 40MHz + SYSPLL + UDIV/2 = **80MHz**；PB22 LED；UART0@PA10/11 115200；SWD |
| CPU frequency | **80000000 Hz**（生成头 `CPUCLK_FREQ` 核对 + 板端 banner 核对） |
| CCS / SDK / SysConfig / compiler | CCS Theia 20.5.1 / MSPM0 SDK 2.10.00.04 / SysConfig 1.27.1+4634 / tiarmclang 4.0.4.LTS |
| board / MCU | 立创天猛星 MSPM0G3507（LQFP-64） |
| debugger | XDS110（外部 SWD，USB 0451:BEF3） |
| build result | **BUILD OK**（`scripts/build.ps1 -Clean`：退出码门禁 + SysConfig 0 error/2 豁免 warning + 编译器 0 error/0 warning + 新 .out） |
| flash result | **FLASH OK**（`scripts/flash.ps1` DSLite 后端：load+verify+run，退出码 0） |
| user board verification | **待用户冷启动×3**（见下） |
| test date | 2026-08-29 |
| UART banner 摘要 | `fw v0.1.0 / app LED_BRINGUP(id=1) / clk CPUCLK=80000000 Hz / rst SYS_DEBUG`（`logs/tmp/p1_80mhz_rst1.txt`） |
| LED result | 心跳运行中（System Reset×1 后目视连续闪烁） |
| known limitations | ① delay 为 `delay_cycles` 忙等（P2 换 TIMG12 节拍）；② UART0 为阻塞发送（P3 换非阻塞队列）；③ banner 每次复位打印一次 |

### 待用户验收：冷启动 ×3（P1 关闭前提）

操作（共 3 轮，每轮独立）：
1. **拔掉板子 TYPE-C 供电（完全断电）→ 等 2 秒 → 重新插上**；
2. 观察并确认：LED 约 1Hz 心跳；串口（COM13，115200）banner **只打印一次**且 `CPUCLK=80000000 Hz`；
3. 记录第 3 轮的 `rst` 字段（断电冷启动应显示 `POR_*` 类，非 `SYS_DEBUG`）。

通过标准：3 轮全部满足 + 全程无电机/舵机/PWM 输出（P1 无执行器代码，物理隔离）。
用户返回结果后，P1 才能标 `COMPLETED`。

---

## 记录 1：32MHz 初始 bringup（历史，**仅适用于 32MHz 旧 commit**）

| 字段 | 值 |
|---|---|
| phase | P1（旧） |
| tested_code_commit | `43e2683`（P1 bringup，32MHz 基线） |
| evidence_record_commit | `43e2683`（同 commit 报告） |
| firmware configuration | 默认 SYSOSC 32MHz；其余同上 |
| CPU frequency | 32000000 Hz |
| build result | 0 编译警告（FLASH 2.5KB / RAM 512B） |
| user board verification | **已通过**（用户目视）：冷启动 LED 1Hz ✓；UART banner 每复位一次 ✓；**按复位键 3 次**（物理复位，非断电冷启动）各打印一次 ✓ |
| test date | 2026-08-28 |
| 适用范围声明 | **仅适用于 32MHz 旧 commit `43e2683`**；32→80MHz 时钟切换使这些结果**不适用于当前版本**（UART 分频器/延时周期均已变）。历史保留，不删除。 |
