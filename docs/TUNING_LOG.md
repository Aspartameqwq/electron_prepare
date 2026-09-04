# 软件策略与整定记录

物理参数见 [HARDWARE_PROFILE](HARDWARE_PROFILE.md)；本文件仅记录软件策略。

## P2 初始策略（2026-09-04）

| 参数 | 编译来源 | 值 | 状态/依据 |
|---|---|---|---|
| 合成任务周期 | project_config.h | 1/10/20ms | DESIGN_DEFAULT；PLAN P2 |
| 测试时长 | project_config.h | 600000ms | DESIGN_DEFAULT；10min |
| 日志周期 | project_config.h | 10000ms | DESIGN_DEFAULT；结束后重发冻结结果 |
| LED 半周期 | project_config.h | 500ms | DESIGN_DEFAULT；1Hz 心跳 |
| 单消息容量 | project_config.h | 512B | DESIGN_DEFAULT；完整复制或拒绝 |
| 每轮 UART0 服务预算 | project_config.h | 16B | DESIGN_DEFAULT；FIFO 满立即返回 |
| 自检等待/最少 tick | project_config.h | 4ms / 2 | DESIGN_DEFAULT；允许启动相位误差 |
| scheduler 最大任务数 | scheduler.h | 16 | DESIGN_DEFAULT；PLAN 固定容量 |
| 最大任务周期 | scheduler.h | INT32_MAX ms | CALCULATED；有符号差值上界 |
| 主栈预算 | firmware/p2_runtime.opt | 2048B | DESIGN_DEFAULT；map 已确认，未实测高水位 |

栈通过 projectspec 将 .opt 作为链接器命令文件加载，位于 SysConfig device_linker.cmd 之后。
不修改生成文件、不重定义存储区域或 NONMAIN。栈是链接期策略，不在 C 配置头重复一个不生效的宏。
P2 不启用 UART1、电机、PI、舵机或传感器，不把未知物理参数写入编译配置。
