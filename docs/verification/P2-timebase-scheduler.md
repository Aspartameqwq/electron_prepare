# P2 时间基准与调度器验证

- 日期：2026-09-04
- 分支：feat/p2-timebase-scheduler
- base_commit：713aec3
- working_tree_dirty：true（测试时）；已归档至 fe943448cd0ec7c958db89fc9ad1615f0598898c
- tested_code_commit：fe943448cd0ec7c958db89fc9ad1615f0598898c（被测源码归档；摘要见下）
- evidence_record_commit：fe943448cd0ec7c958db89fc9ad1615f0598898c（首次归档验收记录；本次仅补 metadata）
- phase_result：COMPLETED
- user_board_verification：PASS（用户确认 LED 及 RESET 测试无异常；执行器未连接）
- 硬件目标：天猛星 MSPM0G3507，LQFP-64；80MHz HFXT+SYSPLL；默认 XDS110

## 软件范围

TIMG12 timebase、集中式 ISR、纯 C11 scheduler、应用分发、10min 合成任务 APP、有界 UART0 诊断及 CCS 接入。
ring/frame 保留预研且不编入 P2 镜像。P1 板端记录只适用于历史版本，不能证明本镜像已上板通过。
接口见 [MODULE_STANDARD](../MODULE_STANDARD.md)，策略见 [TUNING_LOG](../TUNING_LOG.md)。

## 自动验证

| 验证 | 结果 | 边界 |
|---|---|---|
| host C11 -Wall -Wextra -Werror | PASS | ring、frame、scheduler、真实验收 APP + BSP 替身 |
| ASan + UBSan | PASS | 600000ms 回绕仿真、实际顺序、取消/重入、参数边界 |
| APP 正常/漏跑故障注入 | PASS | 正常 RESULT_PASS；跳过 5ms 必须 RESULT_FAIL；停任务并冻结重发 |
| SysConfig 1.27.1 | PASS，有批准警告 | error=0，HFXT 两条精确白名单 |
| CCS clean build | PASS | TI Arm Clang 4.0.4.LTS，C11，-O2，Debug；编译/链接 warning=0 |
| 烧录 | 退出码 0 | XDS110 / DSLite：Program verification successful，load+verify+run |
| 真实板端日志采集 | 完整采集，统计符合预期 | 59 条 RUNNING + 两次相同 RESULT_PASS；用户已确认 LED 与 RESET 测试正常 |

执行入口：

    powershell -ExecutionPolicy Bypass -File scripts/test_host.ps1
    powershell -ExecutionPolicy Bypass -File scripts/test_host.ps1 -Sanitize
    powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Clean
    python scripts/check_doc_consistency.py
    python scripts/check_markdown_links.py
    python scripts/validate_reference_manifest.py

原始日志保存在 gitignored 的 logs/tmp；不提交本机路径、端口号或构建产物。
正式 CCS 构建沿用既有退出码/精确警告集合门禁，无新增警告豁免。

## 构建与源码绑定

镜像为 firmware/Debug/firmware_p1_bringup.out；保留工程标识以兼容 build/flash 脚本，APP_SELECTED_ID=2。
- 镜像 SHA-256：c511d28f32421a0b1fb8e551a6e36fde6ae31fe06f9dd9f97265faf261b4bfbe
- 源码集合 SHA-256：dcda5ba29af37fd1e851e7199869a2ab6da4c515231f0e5317cce0011251a3d6
- 摘要方法：app 全部 .c/.h、board/bsp 全部 .c/.h、config 全部 .h、scheduler.c/.h、frame_codec.h（配置校验依赖）、control.syscfg、projectspec、gate.opt、p2_runtime.opt，按相对路径排序；每项为路径+空格+文件原始字节 SHA-256+LF，再对拼接内容取 SHA-256。明细保存在 logs/tmp/p2_source_manifest.json。
- map：Flash=10568/131072B，SRAM=3719/32768B（含主栈 2048B）；栈高水位未测。
- 固件验证期间未修改参与构建的源码；纯文档修改不改变镜像。

## 生成物复核

- TIMG12：BUSCLK=80000000Hz，divide=1，prescale=0，LOAD=79999，PERIODIC、ZERO，初始 STOP；BSP 在 IRQ 就绪后显式启动。
- tick 位于 4B 对齐 SRAM；真实 TIMG12_IRQHandler 来自 interrupts.c，IIDX 读即确认，hook 只 tick++。
- GPIO_init 早于 SYSCTL_init，Flash 等待状态及 SYSPLL workaround 仍由生成代码提供；TIMER_ERR_01/04/06/07 复核不适用，见 [勘误](../ERRATA_CHECKLIST.md)。
- projectspec 保留源码目录，输出使用构建目录下 syscfg；生成依赖含 GEN_FILES/GEN_OPTS 和 ti_msp_dl_config.o，不手改生成文件。
- p2_runtime.opt 在生成 linker.cmd 之后加载，实际 .stack=0x800；大日志缓冲静态分配，未测栈高水位。

## 用户上板步骤（基础必做）

1. 接好目标板和 XDS110，执行器保持隔离。重新检测探针，确认与 targetConfigs/MSPM0G3507.ccxml 一致，再烧录本次构建产物。
2. 连接板载 CH340 对应 UART0，115200、8N1。端口取本机配置；记录启动及不少于 610 秒的运行日志；另做硬件复位启动观察。若合并进行，则从复位前开始采集。
3. 启动信息应含 app_id=2、CPUCLK=80000000、actuators=disabled；INIT_FAILED 时停止验收并保留日志。观察 LED 约 1Hz 心跳。
4. 每 10s 可见 P2 RUNNING，结束出现 P2 RESULT_PASS：count 接近 600000/60000/30000（各误差≤1），missed=0/0/0，order_errors=0、tick_errors=0；记录 maxlate_ms/report_drops。
5. 再观察一次结果重发：count/missed/elapsed 冻结，LED 继续心跳。若 RESULT_FAIL、异常复位或无进度，保留日志，不标记完成。
6. 用户返回串口日志与 LED 观察结果后，才可登记 timebase 的 BOARD_TESTED、P2=COMPLETED。

计数误差相对 tick 的理论周期数；host 仿真不测物理时钟，手工计时不代表毫秒级频率精度。
外部仪器验证 1ms 精度可另行补充，不伪造测量值。

## 接续位置

P2 基础验收完成：软件验证、10min 自动日志及用户 LED/RESET 口述证据齐备。下一阶段为 P3 核心通信，范围为 UART 日志/transport 与既有 ring/frame 的正式接入及回环验证。
默认不提交、不推送、不合并；不用重做 P1/P1A 规划。

## 本次硬件操作记录

- 沙箱内枚举未看到探针；沙箱外复核实际检测到 XDS110（0451:BEF3）与板载 CH340；与项目 ccxml 一致。
- DSLite 烧录/校验/运行返回 0，完整启动信息已由串口采集：app_id=2、CPUCLK=80000000、reset_code=26。
- 13:07:17（Asia/Shanghai）记录启动；本次为 load+run，不是 POR 或成功执行的 System Reset 验收。
- 随后调用可选 mspm0-ccs DSS 复位辅助脚本在配置调试服务前报 TypeError: ds.setConfig is not a function，退出 1；没有执行该次复位。未改技能或重复中断已运行的计数测试，继续采集原启动的日志。
- 日志为 logs/tmp/p2_board_10min.txt，采集进程退出 0，接收 7921 字节；SHA-256：73a53605ec2ca178cb53076a3f96f12f28d3df94f7dde356db13d9a6a758c85a。
- 13:17:17 首次 RESULT_PASS，13:17:27 再次重发：elapsed_ms=600000，count=600000/60000/30000，missed=0/0/0，maxlate_ms=0/0/0，order_errors=0，tick_errors=0，report_drops=0。
- 自动核对全部 59 条 RUNNING 的 10000ms 连续进度及理论计数，两个最终结果完全一致；启动 banner 仅一次，没有 INIT_FAILED 或 RESULT_FAIL。串口工具按接收块插入时间戳，核对时仅去除该时间戳前缀，原始日志保留。
- 采集结束后重新核对源码集合中每个文件及镜像摘要，均与本记录一致。计数符合固件自身时基预期，不代表仪器测得物理 1ms 精度。
- 用户口述证据（本任务）："led始终保持1Hz闪烁，无异常，未连接舵机/电机"。据此确认 LED 观察正常及执行器未连接；不能据此验证执行器动作或物理时钟精度。
- 用户补充口述证据（本任务）："reset测试过，也无异常"，结合上一条 LED 观察确认硬件复位后运行无异常。未取得这次 RESET 的独立串口日志，不声称复位后另跑了 10min 或完成 POR 测试。
- 依据 PLAN.md 的 P2 基础验收要求，10min 任务统计、host 回绕测试与用户观察证据齐备，登记 P2=COMPLETED，P2 模块为 BOARD_TESTED；执行器功能和仪器级定时精度不在本次结论范围。
