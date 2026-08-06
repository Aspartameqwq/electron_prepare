# ADR-001: Bootstrap 阶段分支策略例外与规则生效

- **状态**：Accepted
- **日期**：2026-08-04
- **决定生效于 commit**：`（本 ADR 所在 commit）`——此后所有修改必须走分支 + PR

## 背景

`AGENTS.md` 规定：`main` 上禁止直接修改，先创建 `feat/<phase>` 分支；未授权不 merge/push；push/PR 需明确授权。但项目早期（P0 引导阶段）多个文档/治理提交直接进入 `main`，无分支、无 PR、无 CI 状态检查。

## 决策

1. **引导阶段例外**：截至本 ADR 所在 commit（`HEAD`），早期 bootstrap 提交直接进入 `main` 属**一次性例外**，不重写历史。
2. **规则正式生效**：**从下一次提交开始**，所有修改必须：
   - 创建 `docs/...` 或 `chore/...` 分支；
   - 提交后创建 PR；
   - 通过 `p0-gate` CI（manifest 校验 / 文档一致性 / Markdown 链接 / host 测试）；
   - 由仓库所有者（Aspartameqwq）自审后合并。
3. **CI 为必需状态检查**：`p0-gate` 设为 `main` 的 required check（GitHub 分支保护）。
4. **不重写历史**：引导期提交保持原样；如需回滚，使用 revert，不 force-push。

## 后果

- 优点：从快照点起具备可追溯、可审查、可回滚的协作流；CI 兜底文档/清单/host 回归。
- 代价：提交成本上升（分支+PR+CI）；早期小修不再直接进 main。
- 若连续多个提交再次绕过分支/PR，P0"分支规则就绪"不得保留为已完成。
