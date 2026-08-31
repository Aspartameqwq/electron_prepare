# ADR-002: 双舵机定时器由 TIMG6 改为 TIMA1

- **状态**：Accepted
- **日期**：2026-08-29（P1A 预检收口时决策；本 ADR 于 P1 final closeout 补记）
- **对应阶段**：P1A（舵机资源在 P7 使用）

## 背景

PLAN.md（v7.2 之前版本）将双舵机 50Hz PWM 分配到 **TIMG6**，并沿用至 P1A 预检。MSPM0G3507 LQFP-64（天猛星板）约束下核查发现：

- TIMG6 可引出的 **CCP0 引脚仅 PA21**，而 PA21 属天猛星 `DO_NOT_USE`（见 PLAN.md §二 / PINMAP.md `DO_NOT_USE` 段）；
- TIMG6 其余可复用引脚 PA0/PA8 分属 I2C0 / TIMA0，无合法双舵机解；
- **TIMA1** 的 CCP0=PA17 / CCP1=PA16 为嘉立创天猛星本板 TB6612/SG90 例程使用过的已验证组合（`examples_and_documents/INDEX.md` 对应条目），且不与 P1A 其他 DRAFT 资源冲突。

`docs/preflight/pin_preflight.syscfg`（NON_BUILDING）经 SysConfig 1.27.1 求解：TIMA1 + PA17/PA16 **0 error**，与其余核心资源共存成立。

## 决策

1. **双舵机 PWM 定时器 = TIMA1**（CCP0=PA17、CCP1=PA16），取代原计划的 TIMG6；
2. TIMG6 不再标为正式舵机定时器；如需使用须重新决策（其 LQFP-64 CCP0 受限问题不因本 ADR 消除）；
3. P7 资源依赖相应更新：`P7 ← TIMA1 资源 + 舵机参数`；
4. **实例为最终决定，本 ADR 不授权改回 TIMG6**（除非出现新的合法引脚方案并另立 ADR）。

依据：预检 solver 数据 + 官方/本板例程引脚组合证据，属**非静默实例变更**（PLAN §一 冲突规则：RESOURCE_MAP↔syscfg 冲突须停止，不得自动换实例；本决策同步落到 PLAN / RESOURCE_MAP / PINMAP / preflight 四处）。

## 后果

- 优点：双舵机资源在板级约束下有合法、已验证的引脚组合；P1A 预检 0 error。
- 代价：TIMA1 原"备用"角色取消，替换为 TIMG7（备用）；后续 P7 实现以 TIMA1 共享定时器语义为准（见 PLAN.md §十）。
- 关联文档同步：`PLAN.md` §三 资源表 / §十 舵机节 / §十二 阶段依赖表、`docs/RESOURCE_MAP.md`（变更记录指向本 ADR）、`docs/PINMAP.md`（PA17/PA16 DRAFT 行）、`docs/preflight/pin_preflight.syscfg`（P7 注释）。