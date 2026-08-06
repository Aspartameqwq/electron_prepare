# STATUS.md — 阶段结果与模块验证状态

> **状态真实性声明（2026-08-04 纠偏）**：项目当前处于 **P0（IN_PROGRESS）** 准备阶段。
> `ring_buffer` / `frame_codec` 是**提前完成的纯软件预研资产**（host 功能测试通过），
> **不代表 P3 已开始**——P3 还缺 `uart1_transport`、调度器接入、板端回环与有界服务。
> 阶段状态词汇：`NOT_STARTED / IN_PROGRESS / COMPLETED / SOFTWARE_READY / BLOCKED / FAILED`。

## 一、阶段结果（phase_result）

| 阶段 | 状态 | 说明 |
|---|---|---|
| P0 | **IN_PROGRESS** | 工具链锁定/硬件档案/host 规范/勘误/默认调试器 XDS110 已就绪；剩余：`detect_probe`（需板子）、**最小 CI（host 测试 / YAML 解析 / 版本一致性 / 绝对路径 / Markdown 链接）** 与 **从下一次提交起强制分支 + PR** 落地。**复杂 CCS 构建 CI 不属于 P0** |
| P1 | **NOT_STARTED** | 尚无 CCS 最小工程 |
| P1A | **NOT_STARTED** | 尚无 `control.syscfg` / 引脚预解算 |
| P2 | **NOT_STARTED** | 尚无 tick / scheduler |
| P3 | **NOT_STARTED** | 尚无 `uart1_transport`、调度接入、板端回环、有界服务 |

## 二、纯软件预研资产（非阶段完成）

| 模块 | 状态 | release_gate | tested/evidence commit | 已知缺口 |
|---|---|---|---|---|
| ring_buffer | HOST_TESTED | **NOT_MET** | `ed63fcf` / `f0ab9c8` | Sanitizer 未启用、**目标编译器并发语义未验证**（volatile≠内存同步，host 仅单线程）、UART transport 集成待定 |
| frame_codec | HOST_TESTED | **NOT_MET** | `ed63fcf` / `f0ab9c8` | 时间戳回绕、Sanitizer 未启用、**空指针/超时参数校验缺失**、`frame_encode` 失败时 `*out_len` 未清零、UART transport 集成待定 |

规则（见 PLAN.md §14）：测试结果只对记录 commit 有效；`BOARD_TESTED` 必须有用户上板证据。
上述两模块**仅为预研**：P3 正式阶段须在 `firmware/` 工程内**接入**、**补齐**边界校验与目标端并发验证、**复核**，并在**目标编译器与板端重新验证**后方可算作阶段完成（**无证据表明现有算法必须整体重写**）。

## 三、当前未存在的关键工件（如实声明）

- **无 CCS 最小工程、无 `control.syscfg`**
- **无 `scheduler`、无 `uart1_transport`**
- **无任何板端测试 / 无上板证据**
- 目录"当前/计划"区分见 `README.md` 第四节；`examples_and_documents/` 为参考库（已入库），`firmware/` 仅含预研纯算法文件，尚未形成可编译工程。
