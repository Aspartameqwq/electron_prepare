# MSPM0G3507 电赛控制类模块库

> 面向 2027 全国大学生电子设计竞赛（控制类）的个人 + 协作者项目。
> 在**天猛星 MSPM0G3507** 上、用 **CCS Theia** 实现可复用、格式规范、逐模块上板验证的控制类模块库。

---

## 一、设计目标

1. **真正把常用控制模块写出来**：不是示例拼贴，而是经"写码 → 编译 → 上板实测"闭环验证的可用驱动。
2. **格式规范、可复用**：统一接口命名、错误枚举、验证状态标注，模块可在电赛现场快速组装。
3. **安全优先**：电机/舵机有硬性安全状态机（上电不转、换向死区、紧急停止同步生效、独立供电）。
4. **个人 AI 辅助 + 协作者协作**：硬件逐步采购、逐步上板；GitHub 分支管理，证据绑定 commit，回滚清晰。
5. **用 skill 规范开发**：`mspm0-ccs` skill 提供 SysConfig/引脚/烧录铁律，所有会话行为一致。

## 二、技术栈与环境

| 项 | 值 |
|---|---|
| 开发板 | LCKFB **天猛星 MSPM0G3507**（LQFP-64） |
| IDE | CCS Theia 20.5.1 |
| SDK | MSPM0 SDK 2.10.00.04 |
| 驱动库 | TI DriverLib + SysConfig |
| 规范层 | 全局 `mspm0-ccs` skill（描述匹配自动触发） |
| 本机路径 | 见 `scripts/env.example.ps1` → 复制为 `env.local.ps1`（已 gitignore） |

## 三、范围分层

```text
核心模块库（发布门槛 = P10 集成冒烟通过）
  UART0 调试日志 · UART1 transport · frame_codec 帧协议 · ring_buffer
  TB6612 电机 · 编码器测速 · 速度 PI · 舵机 PWM
  I2C · SSD1306 OLED · MPU6050 原始 · 基础姿态(roll/pitch/gyro_z/yaw_rel)

可选扩展（不阻塞核心发布）
  E1 K230 会话层      E2 航向控制 + 云台随动
  E3 80MHz 优化       E4 WWDT + 长期可靠性
```

## 四、目录结构

当前（P0 完成，尚未实施模块）：

```
2027-prepare/
├── PLAN.md            # 冻结版实施计划 v7.1（完整规范：安全/协议/验收）
├── README.md          # 本文件（项目中枢）
├── AGENTS.md          # 所有 AI 工具统一强制规则
├── CLAUDE.md          # Claude Code 入口
├── .gitignore         # 排除构建产物/本机配置/临时日志
├── docs/
│   ├── TOOLCHAIN_LOCK.md      # 工具链版本锁定（脱敏）
│   ├── HARDWARE_PROFILE.md    # 硬件参数档案（P0 基础 + 阶段字段 UNKNOWN）
│   ├── HOST_TEST.md           # host 测试规范（clang C11 -Werror）
│   └── ERRATA_CHECKLIST.md    # 芯片勘误清单（按阶段筛选）
└── scripts/env.example.ps1
```

计划目标形态（按阶段逐步创建，不生成空壳）：

```
firmware/               # 唯一 CCS 工程
  control.syscfg        外设配置唯一机器事实源
  app/                  main.c(唯一入口) · app_dispatch.c · tests/
  board/                bsp/ · drivers/ · middleware/ · control/ · estimation/ · safety/
  config/               hardware_config.h · project_config.h · config_validate.h
docs/                   HARDWARE_PROFILE.md · RESOURCE_MAP.md · PINMAP.md
                        PROTOCOL.md · SAFETY.md · TUNING_LOG.md · TOOLCHAIN_LOCK.md
tests/host/             ring/frame_codec/PI/attitude 纯算法 host 测试
scripts/                build.ps1 · flash.ps1 · verify.ps1 · test_host.ps1
```

## 五、开发流程（阶段计划）

**阶段结果**：`COMPLETED`（软硬件均过）· `SOFTWARE_READY`（软件完成等硬件）· `BLOCKED`（缺资源，只阻塞依赖阶段）· `FAILED`（先修复）。

| 阶段 | 内容 | 依赖 |
|---|---|---|
| P0 | 项目骨架（git/AGENTS/CLAUDE/env/TOOLCHAIN_LOCK） | — |
| P1 | 最小工程：单 main、PB22 LED、UART0 启动日志 | P0 |
| P1A | 全资源引脚预解算（pin_preflight.syscfg 非构建） | P1 |
| P2 | 1ms 时间基准 + 调度器（优先级/next_due） | P1 |
| P3 | 核心通信：ring_buffer + uart1_transport + frame_codec | P2 |
| P4 | TB6612 开环电机（安全门禁先行） | P1A 电机资源 |
| P5 | 双编码器 + 自适应测速 | P1A 编码器 GPIO |
| P6 | 速度 PI（软件先行，板上闭环等 P4+P5） | P6-SOFTWARE / P6-BOARD |
| P7 | 双舵机（共享定时器单通道禁用） | TIMG6 资源 |
| P8 | 同步阻塞 I2C + OLED 分块刷新 | I2C0 资源 |
| P9 | MPU6050 + 基础姿态 | I2C 总线过 |
| P10 | **核心集成冒烟**（发布配置下 10min）→ v1.0 | 全核心 |

可选扩展 E1~E4（见 PLAN.md 第十三节）。

## 六、协作指南（协作者必读）

- **分支工作流**：`main` 只放稳定阶段成果；每阶段一个 `feat/pNN-*` 分支、阶段内多个小 commit；阶段结束 PR 合并 `main`，**主理人（Aspartameqwq）自审**。
- **提交规范**：一个 commit 只解决一个问题；**不提交** `Debug/Release`、`*.out/*.obj/*.map`、本机绝对路径、`env.local.ps1`、`logs/tmp/`。
- **验证证据**：任何 `BOARD_TESTED` 状态都必须有证据（串口日志/照片/视频摘要/仪器结果）并绑定 commit；无证据不标上板通过。
- **事实源**：改引脚/外设看 `control.syscfg`；改已确认硬件参数看 `hardware_config.h`；改软件策略看 `project_config.h`；不一致时停止并同步。
- **AI 工具**：任何 Agent 开工前先读 `AGENTS.md` + `PLAN.md`，遵守 git 规则与脏工作树保护。

## 七、回滚指南（快速定位开发要点）

- 每阶段独立分支 + 独立 commit → **回滚到阶段边界 = 切回该阶段分支或 revert 对应 commit**。
- 测试结果只对记录的 commit 有效：`STATUS.md` 记录 `tested_code_commit`（实际烧录测试的代码）与 `evidence_record_commit`（证据入库的 commit）两个字段。
- 回滚示例：
  ```bash
  git log --oneline                     # 找到阶段边界 commit
  git checkout feat/p05-encoder         # 回到某阶段分支
  git revert <commit>                   # 或仅撤销某个问题 commit
  ```
- 发布标签（`v1.0-module-library`）指向**实际被测代码 commit**，Release 附件 .out 与记录的 SHA-256 一致。
- 遇到 BLOCKED/FAILED 时，恢复记录见 `docs/verification/blocked/Pxx-*.md`（含 `base_commit`、`tested_code_commit`(可 N/A)、恢复首条命令）。

## 八、文档索引

| 文件 | 内容 | 何时看 |
|---|---|---|
| `PLAN.md` | 完整规范 v7.1：资源规划、安全状态机、协议、阶段验收、验证矩阵 | 一切开发 |
| `AGENTS.md` | AI 工具强制规则：git、验证边界、脏工作树保护 | 每个 Agent 开工前 |
| `CLAUDE.md` | Claude Code 入口 | Claude Code 会话 |
| `README.md` | 本文件：项目中枢 | 所有人 |
| `docs/TOOLCHAIN_LOCK.md` | 工具链版本锁定（脱敏） | 工具升级/排查 |
| `docs/HARDWARE_PROFILE.md` | 硬件参数档案（分阶段补齐） | 各阶段进入前 |
| `docs/HOST_TEST.md` | host 测试规范 | 纯算法模块测试 |
| `docs/ERRATA_CHECKLIST.md` | 芯片勘误清单（按阶段筛选） | 各阶段开始 |

## 九、当前状态

- **P0 完成**（git 仓库 + 分支规范 + AGENTS/CLAUDE + env 分离 + TOOLCHAIN_LOCK + HARDWARE_PROFILE + host 规范 + 勘误清单 + 默认调试器 XDS110）。
- **P3-SOFTWARE（通信核心纯算法）完成 host 验证**：`ring_buffer`（SPSC）+ `frame_codec`（帧协议 v1，CRC-16/CCITT-FALSE），`scripts/test_host.ps1` 全过（含 CRC 向量 `0x78DA`、1 万随机往返、10 万字节模糊）→ **HOST_TESTED**。
- 调试器决策（官方文档确认）：天猛星**无板载调试器**，**默认调试器 = 外部 XDS110**（SWD: PA19=SWDIO / PA20=SWCLK），J-Link 备用；禁 ST-LINK。
- 待办：插上 XDS110 + 开发板 → `detect_probe.py` 确认 → P1 最小工程；P3 上板（UART1 回环）待硬件。
- 模块验证状态见 `docs/STATUS.md`。
