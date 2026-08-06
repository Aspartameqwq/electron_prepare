# LICENSING_AUDIT.md — 许可审计

> **当前状态（2026-08-04）**：仓库已提交第三方参考资料，但**尚未有根级 `LICENSE`**，部分第三方许可未确认。审计目标：不推测、不越权，逐项确认。

## 一、项目自身许可

- **根目录 `LICENSE`：不存在** → 状态 `PROJECT_LICENSE_DECISION_REQUIRED`。
- **决策权在仓库所有者（Aspartameqwq）**：AI/协作者**不得代为选择开源许可证**。选定前，项目对外呈现为"未声明许可证"。

## 二、第三方组件清单

| 组件 | 来源 | 版本 | 许可状态 | 处置 |
|---|---|---|---|---|
| TI DriverLib | MSPM0 SDK 2.10.00.04 | 2.10.00.04 | SDK 自带（BSD 风格，随 SDK 分发） | 官方提供，**不改动** |
| LCKFB 天猛星模块代码（68 模块） | 嘉立创 wiki | SDK 2.02.00.05 | **`REVIEW_REQUIRED`**——文件头"官网全部开源"≠正式许可证；文档要求复制/传播/修改/公开展示时标明来源与链接 | 已入库仅作参考；**未确认前禁止复制进 `firmware/`** |
| InvenSense DMP（`inv_mpu.c` 等） | MPU6050 例程内 | — | **`LICENSE_MISSING`**——文件头声明"See included License.txt"，但 ZIP 内无该文件 | **禁止复制**；须找回原始 License.txt 并记录 SHA-256 |
| 本项目预研代码（`firmware/middleware/ring_buffer.c`、`frame_codec.c`） | 本项目自写 | — | 本项目自有 | 无第三方成分（纯算法，无外部头文件） |

## 三、处置规则（Agent/协作者必须遵守）

1. **不推测许可**：`REVIEW_REQUIRED` / `LICENSE_MISSING` 未确认前，禁止向 `firmware/` 复制对应代码。
2. **不新增**：未确认公开再分发条件前，禁止向仓库再添加新的第三方文件。
3. **不删除 / 不重写历史**：参考文件保留；迁移（如拆仓）由所有者决定。
4. **保留版权头**：引用第三方实现时不得删除原版权/许可声明；要求附带全文的放入 `licenses/`。
5. 找回 `License.txt` 后：记录来源路径 + SHA-256，更新本审计与 `REFERENCE_MANIFEST.yml`。

## 四、待所有者决策

- 项目根许可证选择（`PROJECT_LICENSE_DECISION_REQUIRED`）。
- LCKFB 模块代码再分发确认（联系立创/查看 wiki 许可条款）。
- InvenSense `License.txt` 找回。
- 是否拆仓：主仓库只存目标代码/计划/评审/来源清单，参考资料独立仓库。
