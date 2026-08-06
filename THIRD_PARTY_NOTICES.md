# THIRD_PARTY_NOTICES.md — 第三方声明

> 本文件记录所有第三方软件/文档成分。**精确来源 + 固定归属格式**；详细分层审计见 `docs/LICENSING_AUDIT.md`，来源哈希见 `docs/reference/REFERENCE_MANIFEST.yml`。

## 固定归属格式

引用下列任何内容时，须保留此格式：
> 来源：立创开发板（[www.lckfb.com](https://www.lckfb.com)）· 天猛星 MSPM0G3507 开发板 —— {具体资料名}，文档：<https://wiki.lckfb.com/zh-hans/tmx-mspm0g3507/>

TI 内容：
> 来源：Texas Instruments —— {文档/示例名}，{文档编号/版本}

## 第三方成分（按来源精确登记）

### 1. LCKFB 天猛星模块移植代码（源码）

| 模块 | 精确页面 | 内容类型 | 状态 |
|---|---|---|---|
| TB6612 / N20 编码器 / SG90 等 68 个模块 | `wiki.lckfb.com/zh-hans/tmx-mspm0g3507/module/`（模块手册；每模块精确页见 wiki 目录） | 源码（CCS 工程 + BSP 驱动） | `REVIEW_REQUIRED` |

### 2. LCKFB 板级资料（文档/原理图/手册）

| 资料 | 精确页面 | 内容类型 | 状态 |
|---|---|---|---|
| 引脚图 / 原理图 | `wiki.lckfb.com/zh-hans/tmx-mspm0g3507/`（开发板介绍 + 开源硬件） | 原理图 / PDF | `REVIEW_REQUIRED` |
| CCS-Theia 入门手册 / Keil 手册 / 模块移植手册 / 常见问题 | `wiki.lckfb.com/zh-hans/tmx-mspm0g3507/` | 文档 HTML | `REVIEW_REQUIRED` |
| 数据手册 / 用户手册 / 硬件手册 | TI（见下）| PDF | `REVIEW_REQUIRED` |

### 3. TI 文档与 SDK

| 成分 | 精确编号/版本 | 内容类型 | 状态 |
|---|---|---|---|
| MSPM0G350x 数据手册 | slasex6*（以仓库内 PDF 为准） | 数据手册 PDF | 官方分发 |
| MSPM0G350x 用户手册 / MSPM0G 系列硬件手册 | 见仓库内 PDF 编号 | PDF | 官方分发 |
| MSPM0 SDK 2.10.00.04 | 许可文件 `license_mspm0_sdk_2_10_00_04.txt`（SDK 根目录） | 软件 + 许可 | 官方分发（**不并入仓库**） |

### 4. InvenSense DMP（嵌套第三方）

| 成分 | 来源 | 内容类型 | 状态 |
|---|---|---|---|
| `inv_mpu.c` / `inv_mpu_dmp_motion_driver.c` | MPU6050 例程内含（TDK InvenSense） | 源码 | `LICENSE_MISSING`（ZIP 内无 License.txt）——**禁止复制** |

### 5. 字库 / 图片资源

| 成分 | 来源 | 内容类型 | 状态 |
|---|---|---|---|
| OLED 例程 `oledfont.h` / `bmp.h` 字模数组 | LCKFB 例程随附 | 资源数组 | `REVIEW_REQUIRED`——**未确认前不得复制进 firmware/** |

## 本项目自研（无第三方成分）

- `firmware/middleware/ring_buffer.{c,h}`、`frame_codec.{c,h}`、`firmware/config/*`、`tests/host/*`——纯算法自研，无外部头文件依赖。

## 声明与维护

- 未取得正式许可前，`firmware/` 内**不得**包含第三方实现代码/字库数组。
- 任何从参考例程借鉴的协议/寄存器正确性，均须在本项目接口内**重新实现**，并在本文件登记来源。
- 原代码要求保留的版权头**不得删除**；要求附带许可全文的放入 `licenses/`。
- 每次引用新的第三方内容 → 更新本文件 + `REFERENCE_MANIFEST.yml` + `LICENSING_AUDIT.md`。
