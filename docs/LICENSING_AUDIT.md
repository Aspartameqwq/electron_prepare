# LICENSING_AUDIT.md — 许可审计

> **当前状态（2026-08-04）**：仓库已提交第三方参考资料，但**尚无根级 `LICENSE`**，部分第三方许可未确认。审计目标：不推测、不越权，逐项按内容归属分层确认。

## 一、项目自身许可

- **根目录 `LICENSE`：不存在** → 状态 `PROJECT_LICENSE_DECISION_REQUIRED`。
- **决策权在仓库所有者（Aspartameqwq）**：AI/协作者**不得代为选择开源许可证**。选定前，项目对外呈现为"未声明许可证"。

## 二、第三方组件分层清单

> 按 `content_owner` 分层（不把 LCKFB 内容整体混为一谈）。字段：`content_owner / usage_terms / attribution_required / nested_third_party / nested_license_status / redistribution_status`。

### 1. LCKFB 自有源码与文档（模块移植代码、板级资料）

- **content_owner**：嘉立创（立创开发板，www.lckfb.com）
- **usage_terms**：文件头注明"官网全部开源"；wiki 要求复制/传播/修改/公开展示时**标明来源与链接**（`wiki.lckfb.com`）。**非正式开源许可证文本** → `REVIEW_REQUIRED`
- **attribution_required**：是（标明来源与链接）
- **nested_third_party**：见下文各例程内含的厂商参考代码（如 InvenSense DMP）
- **nested_license_status**：MPU6050 例程内含 InvenSense DMP → `LICENSE_MISSING`（见第 3 条）
- **redistribution_status**：`REVIEW_REQUIRED`

### 2. TI SDK / 生成文件

- **content_owner**：Texas Instruments
- **usage_terms**：随 MSPM0 SDK 分发的许可文件 → **`F:\TI\mspm0_sdk_2_10_00_04\license_mspm0_sdk_2_10_00_04.txt`**（不在仓库内，仅记录路径与文件名；不要用"BSD 风格"这类非精确描述）
- **attribution_required**：遵循该许可文件要求
- **nested_third_party**：无
- **nested_license_status**：—（随 SDK 官方分发）
- **redistribution_status**：`REVIEW_REQUIRED`（是否随本项目再分发由所有者决定；本项目不把 SDK 代码并入仓库）

### 3. 芯片厂商参考代码（InvenSense DMP）

- **content_owner**：InvenSense（TDK）
- **usage_terms**：`inv_mpu.c` 文件头声明 `See included License.txt`，但**解压目录与原始 ZIP 内均无该文件** → `LICENSE_MISSING`
- **attribution_required**：未知（无许可文本可依）
- **nested_third_party**：无
- **nested_license_status**：`LICENSE_MISSING`
- **redistribution_status**：`REVIEW_REQUIRED`；**未确认前禁止复制该代码**

### 4. 字库、图片等资源（OLED 例程 `oledfont.h`、`bmp.h`）

- **content_owner**：LCKFB（例程随附）
- **usage_terms**：同第 1 条 `REVIEW_REQUIRED`
- **attribution_required**：是
- **nested_third_party**：未知（字模可能源自常见库）
- **nested_license_status**：`REVIEW_REQUIRED`
- **redistribution_status**：`REVIEW_REQUIRED`；**未确认前不得把字库数组复制进 `firmware/`**

## 三、处置规则（Agent/协作者必须遵守）

1. **不推测许可**：`REVIEW_REQUIRED` / `LICENSE_MISSING` 未确认前，禁止向 `firmware/` 复制对应代码/资源。
2. **不新增**：未确认公开再分发条件前，禁止向仓库再添加新的第三方文件。
3. **不删除 / 不重写历史**：参考文件保留；迁移（如拆仓）由所有者决定。
4. **保留版权头**：引用第三方实现时不得删除原版权/许可声明；要求附带全文的放入 `licenses/`。
5. 找回 `License.txt` 后：记录来源路径 + SHA-256，更新本审计与 `REFERENCE_MANIFEST.yml`。

## 四、待所有者决策

- 项目根许可证选择（`PROJECT_LICENSE_DECISION_REQUIRED`）。
- LCKFB 内容再分发确认（联系立创 / 查看 wiki 许可条款）。
- InvenSense `License.txt` 找回。
- OLED 字库数组来源与许可确认。
- 是否拆仓：主仓库只存目标代码/计划/评审/来源清单，参考资料独立仓库。
