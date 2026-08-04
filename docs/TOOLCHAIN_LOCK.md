# TOOLCHAIN_LOCK.md — 工具链锁定（脱敏）

本文件记录开发工具链的锁定版本。**已脱敏**：不含本机路径、盘符、COM 口、机器名。
原始命令输出存 `logs/tmp/toolchain/`（已 gitignore），不进入仓库。

检查日期：2026-08-04

| 工具 | 版本 | SHA-256 | 说明 |
|---|---|---|---|
| CCS Theia | 20.5.1.00012 | — | 安装包 `ccs_setup_20.5.1.00012` |
| TI Arm Clang | 4.0.4.LTS | `9e098f8d5708368ec198df898201c5cc561419b33cbd765dae033146da1f449b` | 目标 `arm-ti-none-eabi` |
| SysConfig | 1.27.1+4634 | — | 随 CCS 安装 |
| MSPM0 SDK | 2.10.00.04 | — | 本地 SDK（2.10 官方验证组合需 SysConfig ≥1.26） |
| DSLite | 20.5.0.4063 | `9d1f3ac152ceae57de745e7f7e0448e04b5f5fb7853706a00c193ddbd6df2c12` | 操作：`load / flash / memory / identifyProbe / help` |
| Python | 3.12.4 | — | 运行 mspm0-ccs skill 脚本 |
| Host clang | 19.1.3 | — | host 测试编译器（`-Werror`） |
| Host gcc | 13.2.0 (MSYS2) | — | host 测试备用编译器 |

## DSLite 命令形态（本机确认）

- 全局参数形式：`DSLite [operation] [Args...]`；不支持 `--help`，帮助用 `DSLite help`。
- 烧录/调试前先确认探针：`DSLite identifyProbe` 或 skill 脚本 `detect_probe.py`。
- 完整参数以 `DSLite help` 输出为准（原始输出存 `logs/tmp/toolchain/dslite_help.txt`）。**不得凭记忆使用未在本机确认的参数组合。**

## 更新与校验规则

1. 任何工具升级后：更新本表 + 重新采集原始输出到 `logs/tmp/toolchain/`。
2. 版本号以实际命令输出为准，不手工填写。
3. 本文件只保留脱敏信息；发现路径/COM/机器名立即移除。
