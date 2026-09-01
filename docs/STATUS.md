# STATUS.md — 阶段结果与模块验证状态

> **状态真实性声明（2026-08-04）**：**P0 已 COMPLETED**（探针证据 + 治理闭环均落地）。
> `ring_buffer` / `frame_codec` 是**提前完成的纯软件预研资产**（host 功能测试通过），
> **不代表 P3 已开始**——P3 还缺 `uart1_transport`、调度器接入、板端回环与有界服务。
> 阶段状态词汇：`NOT_STARTED / IN_PROGRESS / COMPLETED / SOFTWARE_READY / BLOCKED / FAILED`。

## 一、阶段结果（phase_result）

| 阶段 | 状态 | 说明 |
|---|---|---|
| P0 | **COMPLETED**（2026-08-04） | 工具链锁定 / 硬件档案 / host 规范（clang C11 + Sanitizer）/ 勘误 / 默认调试器 XDS110 / **探针证据**（`detect_probe`=XDS110 0451:BEF3（Aux/App 两个 UART 口）；DAP 连接+寄存器读取成功，`logs/tmp/toolchain/probe_connect.txt`）/ **治理闭环**（manifest 81 项+校验脚本+`p0-gate` CI+ADR-001 分支策略，经 PR #1 真实合并）/ host 核心加固（PR #2）。可选（不阻塞）：GitHub 端将 `p0-gate` 设为 main required check |
| P1 | **COMPLETED**（2026-09-01 用户 POR 冷启动×3 验收通过） | 最小工程 `firmware/`（`control.syscfg` **80MHz 正式基线** HFXT+SYSPLL + PB22 LED + UART0 + SWD；唯一 `app/main.c`）；CCS headless 构建 0 编译警告（compiler `-Werror` + linker `--emit_warnings_as_errors` 工具链强制，2026-09-01）；XDS110 烧录成功（program verify OK，main @4ef5cef 产物 2026-09-01 复验）；**80MHz 板端自动验证＝XDS110 System Reset ×3 + main 复烟**（banner `CPUCLK=80000000 Hz` 每轮一次、UART/LED 正常，`logs/tmp/p1_80mhz_cold1/2/3.txt`、`p1_main_smoke.txt`；**System Reset≠POR**）。**P1 使用功能勘误已全量筛查闭环**：SYSPLL_ERR_01/HANDLED_BY_SDK（FCC workaround 默认开启）、Flash 80MHz 等待状态/HANDLED_BY_SDK、IOMUX_ERR_02/HANDLED_BY_SDK（生成 init 顺序使唯一 PINCM RMW 在 MCLK≤40MHz 完成）、UART/GPIO/时钟路径按 SLAZ742H 明确 NOT_RELEVANT（依据见 `docs/ERRATA_CHECKLIST.md`）。**用户 POR 冷启动 ×3 通过（2026-09-01 口述证据：每轮完全断电→上电、LED 1Hz 心跳、banner 每轮一次、CPUCLK=80000000、无执行器输出）**→ `user_board_verification=PASS`、P1=COMPLETED（详见 `docs/verification/P1-bringup.md`） |
| P1A | **COMPLETED**（2026-08-29） | 全资源预解算通过：`docs/preflight/pin_preflight.syscfg`（NON_BUILDING）SysConfig **0 error**/7 warning（书面豁免见 ERRATA）；**TIMG12 可分配 ✓**（P2 门禁）；舵机定时器 TIMG6→**TIMA1**（依据预检数据+官方例程证据，决策记录 `docs/decisions/ADR-002-servo-timer-tima1.md`，四文档已同步）；`docs/RESOURCE_MAP.md` + `docs/PINMAP.md`（DRAFT 17 引脚）已建 |
| P2 | **NOT_STARTED** | 尚无 tick / scheduler |
| P3 | **NOT_STARTED** | 尚无 `uart1_transport`、调度接入、板端回环、有界服务 |

## 二、纯软件预研资产（非阶段完成）

| 模块 | 状态 | release_gate | tested/evidence commit | 已知缺口（剩余） |
|---|---|---|---|---|
| ring_buffer | HOST_TESTED | **NOT_MET** | `ed63fcf` / `f0ab9c8` | **目标编译器并发语义未验证**（volatile≠内存同步，host 仅单线程）、UART transport 集成待定 |
| frame_codec | HOST_TESTED | **NOT_MET** | `ed63fcf` / `f0ab9c8` | UART transport 集成待定 |

> **host 加固（2026-08-04，分支 `chore/p0-host-hardening`）**：ring_buffer/frame_codec 已补**空指针/配置校验**（`frame_parser_init` 拒绝非法超时并返回 bool）、`frame_encode` 失败**清 `*out_len`**、**UINT32_MAX 回绕 / 多帧连续流 / 错误后恢复 / 阈值精确边界** 测试；Sanitizer 通过 `scripts/test_host.ps1 -Sanitize`（ASan/UBSan）运行。**`release_gate` 仍 `NOT_MET`**：目标端并发语义（SPSC 发布顺序须在目标编译器 + 板端验证）与 UART transport 集成待定。

规则（见 PLAN.md §14）：测试结果只对记录 commit 有效；`BOARD_TESTED` 必须有用户上板证据。
上述两模块**仅为预研**：P3 正式阶段须在 `firmware/` 工程内**接入**、**补齐**边界校验与目标端并发验证、**复核**，并在**目标编译器与板端重新验证**后方可算作阶段完成（**无证据表明现有算法必须整体重写**）。

## 三、关键工件现状（如实声明，2026-08-29）

**已存在**：CCS 最小工程 `firmware/`（`control.syscfg` 80MHz 基线 + `app/main.c` + projectspec + gate.opt）；P1 板端证据（32MHz 历史；80MHz System Reset×3 + main 复烟 + **用户 POR 冷启动×3 通过 2026-09-01，P1 COMPLETED**）；P1 使用功能勘误全量筛查闭环（见 ERRATA_CHECKLIST）；`docs/RESOURCE_MAP.md` / `PINMAP.md`（DRAFT）/ `docs/decisions/` ADR-001/002。
**仍不存在**：`scheduler`/timebase、`uart1_transport`、P2/P3 全部内容（P2 于 `feat/p2-timebase-scheduler` 分支开始）。
目录"当前/计划"区分见 `README.md` 第四节；`examples_and_documents/` 为参考库（已入库）。
