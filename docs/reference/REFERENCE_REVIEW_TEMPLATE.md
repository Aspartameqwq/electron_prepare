# REFERENCE_REVIEW_TEMPLATE.md — 参考例程评审模板

> 每份例程评审必须按此模板组织，**不得把源码事实、计算推导、硬件猜测与本项目设计决策写在同一类结论中**。
> 核心原则：**"源码确实这样写" ≠ "这种写法正确或值得借鉴"** —— 每条结论须同时给出"证据分类"与"正确性状态"两个维度。

## 必填字段

| 字段 | 说明 |
|---|---|
| `source_id` | 稳定标识，见 `docs/reference/REFERENCE_MANIFEST.yml` |
| 原始归档路径 | 仓库内相对路径（ZIP） |
| 原始归档 SHA-256 | 与 REFERENCE_MANIFEST 一致 |
| SDK / SysConfig 版本 | 例程 `empty.syscfg` 的 `@product` / `@versions` |
| 评审日期 | 日期 |
| 许可证状态 | `OK / REVIEW_REQUIRED / LICENSE_MISSING` |

## 证据分类（classification — 结论是怎么来的）

| 分类 | 含义 |
|---|---|
| `SOURCE_FACT` | 源码 / `.syscfg` 可直接引用的事实。**只证明代码如此写，不代表写法正确或值得借鉴** |
| `CALCULATED` | 计算推导，须标 `VERIFIED / UNVERIFIED` |
| `INFERRED` | 推断，不得当事实 |
| `HARDWARE_CONFIRMATION_REQUIRED` | 须查原理图/万用表/示波器确认，未确认前禁止采信 |
| `TARGET_PROJECT_DECISION` | 本项目设计决策（与例程事实无关） |

## 正确性状态（correctness — 这条结论可不可用）

| 状态 | 含义 |
|---|---|
| `UNVERIFIED` | 尚未验证正确性 |
| `SPEC_VERIFIED` | 与芯片数据手册 / 官方规范核对一致 |
| `HARDWARE_REQUIRED` | 需上板 / 仪器确认 |
| `REJECTED` | 判定为缺陷，弃用 |
| `PROJECT_DECISION` | 本项目选定的方案（非例程评价） |

## 证据表

每项结论用 `finding_id` 编号，两列并存：

| finding_id | classification | correctness | source_file | symbol_or_config | evidence | confidence |
|---|---|---|---|---|---|---|
| … | … | … | … | … | … | HIGH/MED/LOW |

## 结论分区（强制）

1. **可取之处**（`SOURCE_FACT` + `SPEC_VERIFIED`，说明如何借鉴）
2. **不足之处 / 弃用**（`SOURCE_FACT` + `REJECTED` 的缺陷，`CALCULATED`/`INFERRED` 疑点须分开标注）
3. **待硬件确认内容**（`HARDWARE_CONFIRMATION_REQUIRED`）
4. **对应目标接口**（本项目 `firmware/` 中的模块与接口）
5. **后续验收测试**（P 阶段与验收项）

## 铁律

- TI DriverLib（`source/ti/driverlib/`）：供应商源码，**不修改**；
- `ti_msp_dl_config.c/h`：SysConfig 生成产物，**不手工编辑**，但可通过修改 `.syscfg` 后重新生成；
- `DL_*`：API 名称，不是文件；
- 本项目适配写在自有 BSP / driver 层。
- `LICENSE_MISSING` / `REVIEW_REQUIRED` 的代码**禁止复制**。
