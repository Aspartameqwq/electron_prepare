# P1 Bringup 验证记录

> 正式轻量验证记录（大日志在 `logs/tmp/`，gitignored；本记录只留脱敏摘要）。
> 测试结果只对记录的 `tested_code_commit` 有效；新代码使旧结果失效时**不删除历史**，只标注适用范围。
> **commit 语义**（PLAN.md §14）：`tested_code_commit`＝被构建/烧录/测试的代码 commit；`evidence_record_commit`＝**首次把对应测试证据摘要持久化进仓库的 commit**（可与 tested_code_commit 相同或不同；禁止在 commit 自身内容里保存自己的 SHA——证据 metadata 修正不得把该字段改成修正 commit 自身）。
> 验证边界（PLAN.md §14）：`automated_reset_verification`＝Agent 经调试器执行的自动复位验证；`user_board_verification`＝用户物理断电冷启动验收。两者不混称。

## 记录 2：80MHz 基线（当前版本）

| 字段 | 值 |
|---|---|
| phase | P1 |
| tested_code_commit | `f39f61a4855b811f9a77251c57056792220176f1`（本轮 closeout 收口 commit；其工作树即 2026-08-31 clean build + DSLite flash + reset smoke 的代码；后续 commit 均为 docs/scripts-only，不改 firmware 二进制） |
| evidence_record_commit | `eb04ec5df300b5e677c725b89572faf68b7681c7`（**首次**把该测试证据摘要（含 tested SHA、2026-08-31 closeout smoke）持久化进仓库的 commit；与 tested_code_commit 不同——`f39f61a` 自身版本仍为 placeholder） |
| firmware configuration | `control.syscfg`：HFXT 40MHz + SYSPLL + UDIV/2 = **80MHz**；PB22 LED；UART0@PA10/11 115200；SWD |
| CPU frequency | **80000000 Hz**（生成头 `CPUCLK_FREQ` 核对 + 板端 banner 核对） |
| CCS / SDK / SysConfig / compiler | CCS Theia 20.5.1 / MSPM0 SDK 2.10.00.04 / SysConfig 1.27.1+4634 / tiarmclang 4.0.4.LTS |
| board / MCU | 立创天猛星 MSPM0G3507（LQFP-64） |
| debugger | XDS110（外部 SWD，USB 0451:BEF3） |
| build result | **BUILD OK**（`scripts/build.ps1 -Clean`：退出码门禁 + SysConfig 0 error/2 warning 白名单精确匹配 + 编译器 0 error/0 warning + 新 .out） |
| flash result | **FLASH OK**（`scripts/flash.ps1` DSLite 后端：load+verify+run，退出码 0） |
| warnings-as-errors 门禁（2026-09-01） | compiler `-Werror` 经 `firmware/gate.opt` 响应文件传入（CCS TICLANG 工程模型丢弃 projectspec 里的裸 `-Werror`；@file 由 tiarmclang 自身展开）；linker `--emit_warnings_as_errors`（tiarmlnk help 确认）经 `-Wl,` 透传。**实证**：① 单独工具链负测试（warning+`-Werror`→error 退出 1）；② 端到端负测试——临时在 gate.opt 加 `-Wnotarealoption` → 构建失败、无 .out（diagnostics `[-Werror,-Wunknown-warning-option]` 证明两选项同时生效）→ 已还原；③ 最终 clean build BUILD OK（SysConfig error=0/warning=2 exact-set、compiler+linker 0/0、fresh .out）。**本项为 build tooling 变更（projectspec/gate.opt），不改生成代码与固件源；tested commit 仍 f39f61a** |
| final pipeline flash/smoke（2026-09-01） | **PASS（main 合并后补做）**：探针重连后，main @ `4ef5cef` 的 clean build 产物（构建于 merged main，树与 f39f61a 固件源逐字节同源）→ DSLite flash **exit 0 + "Program verification successful"** → XDS110 **System Reset** 冒烟（`logs/tmp/p1_main_smoke.txt`：banner 恰好一次、`CPUCLK=80000000 Hz`、`rst SYS_DEBUG`、target left running；用户同步目视 LED 1Hz 心跳）。**注意：System Reset 是调试复位，非 POR 冷启动**；POR ×3 见 `user_board_verification` |
| automated_reset_verification | **通过（2026-08-29，Agent 经 XDS110 执行）**：System Reset ×3（`logs/tmp/p1_80mhz_cold1/2/3.txt`）——每轮 banner 恰好一次、`CPUCLK=80000000 Hz` 一致、UART 正常、LED 1Hz 心跳、无执行器输出（P1 无执行器代码）。**2026-08-31 closeout 复验（f39f61a）：clean build → DSLite flash（program verification OK）→ XDS110 System Reset 冒烟**（`logs/tmp/p1_closeout_smoke.txt`：banner 恰好一次、`CPUCLK=80000000 Hz`、`rst SYS_DEBUG`、target left running）。**注意：System Reset 是调试复位（rst=SYS_DEBUG 可证），非 POR 物理断电冷启动** |
| user_board_verification | **PENDING**：POR 级物理断电冷启动 ×3 尚未执行（操作与通过标准见下节"待用户验收"）；Agent 不得把 System Reset 描述为 cold boot |
| test dates | initial automated reset verification = **2026-08-29**（System Reset ×3）；closeout re-verification = **2026-08-31**（f39f61a clean build + DSLite flash + reset smoke）；warnings-as-errors build-gate 验证 = **2026-09-01**（clean build PASS + 负测试 PASS）；final pipeline flash/smoke（main @4ef5cef）= **2026-09-01 PASS**；user POR verification = **PENDING**（待用户） |
| UART banner 摘要 | `fw v0.1.0 / app LED_BRINGUP(id=1) / clk CPUCLK=80000000 Hz / rst SYS_DEBUG`（`logs/tmp/p1_80mhz_rst1.txt`） |
| LED result | 心跳运行中（System Reset ×3 每轮后均连续闪烁） |
| SYSPLL_ERR_01 workaround 记录 | SDK/SysConfig 处理机制：SLAZ742H 官方 workaround＝FCC（Frequency Clock Counter）监测 SYSPLL 频率，错误则 disable/re-enable 重锁。SysConfig SYSCTL 时钟选项 `enableWorkaround_SYSPLL_ERR_01` **默认 true**（displayName "Validate SYSPLL Frequency Lock"；`isdeviceAffected_SYSPLL_ERR_01()` 全设备 true）→ 生成代码 `firmware/Debug/syscfg/ti_msp_dl_config.c` L108-199 自动含 FCC 测频（LFCLK 触发）+ 比例界检查（`FCC_EXPECTED_RATIO=2000`＝80MHz/40MHz，±0.3%）+ 失败 toggle SYSPLL 重锁循环（注释 `[SYSPLL_ERR_01]`）。本项目未覆盖该 WEAK 函数、未改生成文件。详见 `docs/ERRATA_CHECKLIST.md` |
| known limitations | ① delay 为 `delay_cycles` 忙等（P2 换 TIMG12 节拍）；② UART0 为阻塞发送（P3 换非阻塞队列）；③ banner 每次复位打印一次 |

### 待用户验收：冷启动 ×3（P1 关闭前提，user_board_verification=PENDING）

操作（共 3 轮，每轮独立）：
1. **拔掉板子 TYPE-C 供电（完全断电）→ 等 2 秒 → 重新插上**；
2. 观察并确认：LED 约 1Hz 心跳；串口（local SERIAL_PORT，115200）banner **只打印一次**且 `CPUCLK=80000000 Hz`；
3. 记录第 3 轮的 `rst` 字段（断电冷启动应显示 `POR_*` 类，非 `SYS_DEBUG`）。

通过标准：3 轮全部满足 + 全程无电机/舵机/PWM 输出（P1 无执行器代码，物理隔离）。
用户返回以上证据后，`user_board_verification` 才标通过、P1 才能标 `COMPLETED`。

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