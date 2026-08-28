# STATUS.md — 阶段结果与模块验证状态

> **状态真实性声明（2026-08-04）**：**P0 已 COMPLETED**（探针证据 + 治理闭环均落地）。
> `ring_buffer` / `frame_codec` 是**提前完成的纯软件预研资产**（host 功能测试通过），
> **不代表 P3 已开始**——P3 还缺 `uart1_transport`、调度器接入、板端回环与有界服务。
> 阶段状态词汇：`NOT_STARTED / IN_PROGRESS / COMPLETED / SOFTWARE_READY / BLOCKED / FAILED`。

## 一、阶段结果（phase_result）

| 阶段 | 状态 | 说明 |
|---|---|---|
| P0 | **COMPLETED**（2026-08-04） | 工具链锁定 / 硬件档案 / host 规范（clang C11 + Sanitizer）/ 勘误 / 默认调试器 XDS110 / **探针证据**（`detect_probe`=XDS110 0451:BEF3 COM11/12；DAP 连接+寄存器读取成功，`logs/tmp/toolchain/probe_connect.txt`）/ **治理闭环**（manifest 81 项+校验脚本+`p0-gate` CI+ADR-001 分支策略，经 PR #1 真实合并）/ host 核心加固（PR #2）。可选（不阻塞）：GitHub 端将 `p0-gate` 设为 main required check |
| P1 | **NOT_STARTED** | 尚无 CCS 最小工程 |
| P1A | **NOT_STARTED** | 尚无 `control.syscfg` / 引脚预解算 |
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

## 三、当前未存在的关键工件（如实声明）

- **无 CCS 最小工程、无 `control.syscfg`**
- **无 `scheduler`、无 `uart1_transport`**
- **无任何板端测试 / 无上板证据**
- 目录"当前/计划"区分见 `README.md` 第四节；`examples_and_documents/` 为参考库（已入库），`firmware/` 仅含预研纯算法文件，尚未形成可编译工程。
