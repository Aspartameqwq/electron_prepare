# THIRD_PARTY_NOTICES.md — 第三方声明

> 本文件记录所有第三方软件/文档成分。详细审计见 `docs/LICENSING_AUDIT.md` 与 `docs/reference/REFERENCE_MANIFEST.yml`。

## 第三方成分

| 成分 | 来源 | 状态 | 位置 |
|---|---|---|---|
| TI MSPM0 DriverLib | TI MSPM0 SDK 2.10.00.04 | SDK 自带许可，**未修改** | 本地 SDK `source/ti/driverlib/`（不在仓库内） |
| LCKFB 天猛星模块移植代码（68 模块） | 嘉立创 wiki | `REVIEW_REQUIRED`（许可未正式确认） | `examples_and_documents/立创·天猛星MSPM0G3507开发板【模块移植代码】/` |
| LCKFB 板级资料（引脚图/原理图/手册/例程） | 嘉立创 / TI | `REVIEW_REQUIRED` | `examples_and_documents/立创·天猛星MSPM0G3507开发板资料/` |
| InvenSense DMP（`inv_mpu.c` / `inv_mpu_dmp_motion_driver.c`） | MPU6050 例程内含 | `LICENSE_MISSING`（ZIP 内无 License.txt） | 同上（传感器类/MPU6050 例程内） |

## 本项目自研（无第三方成分）

- `firmware/middleware/ring_buffer.{c,h}`、`frame_codec.{c,h}`、`firmware/config/*`、`tests/host/*`——纯算法自研，无外部头文件依赖。

## 声明

- 未取得正式许可前，`firmware/` 内**不得**包含第三方实现代码。
- 任何从参考例程借鉴的协议/寄存器正确性，均须在本项目接口内**重新实现**，并在本文件登记来源。
- 原代码要求保留的版权头**不得删除**；要求附带许可全文的放入 `licenses/`。

## 维护

- 每次引用新的第三方代码 → 更新本文件 + `REFERENCE_MANIFEST.yml` + `LICENSING_AUDIT.md`。
