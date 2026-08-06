# REFERENCE_REVIEW_TEMPLATE.md — 参考例程评审模板

> 每份例程评审必须按此模板组织，**不得把源码事实、计算推导、硬件猜测与本项目设计决策写在同一类结论中**。

## 必填字段

| 字段 | 说明 |
|---|---|
| `source_id` | 稳定标识，见 `docs/reference/REFERENCE_MANIFEST.yml` |
| 原始归档路径 | 相对仓库路径（zip） |
| 原始归档 SHA-256 | 见 REFERENCE_MANIFEST |
| SDK 版本 | 例程 `empty.syscfg` 的 `@product` |
| SysConfig 版本 | 例程 `@versions` |
| 许可证文件和许可状态 | `OK / REVIEW_REQUIRED / LICENSE_MISSING` |
| 评审对应版本 | 归档 SHA-256（短）+ 评审日期 |

## 证据表

每项结论用 `finding_id` 编号，`classification` 只能取：

| 分类 | 含义 | 可用性 |
|---|---|---|
| `SOURCE_FACT` | 可直接引用源码 / `.syscfg` 的事实 | 可直接借鉴（仍须按本项目重写） |
| `CALCULATED` | 计算推导 | 须标 `VERIFIED / UNVERIFIED` |
| `INFERRED` | 推断 | 不得当事实 |
| `HARDWARE_CONFIRMATION_REQUIRED` | 须查原理图/万用表/示波器确认 | 未确认前禁止采信 |
| `TARGET_PROJECT_DECISION` | 本项目设计决策（非例程事实） | 与例程无关 |

| finding_id | classification | source_file | symbol_or_config | evidence | confidence |
|---|---|---|---|---|---|
| … | … | … | … | … | HIGH/MED/LOW |

## 结论分区（强制）

1. **可取之处**（仅 `SOURCE_FACT`，且说明如何借鉴）
2. **不足之处 / 弃用**（`SOURCE_FACT` 缺陷 + `CALCULATED`/`INFERRED` 疑点须分开标注）
3. **待硬件确认内容**（`HARDWARE_CONFIRMATION_REQUIRED`）
4. **对应目标接口**（本项目 `firmware/` 中的模块与接口）
5. **后续验收测试**（P 阶段与验收项）

## 铁律

TI DriverLib 与生成文件（`ti_msp_dl_config.*`）**绝不允许修改**；`LICENSE_MISSING` / `REVIEW_REQUIRED` 的代码**禁止复制**。
