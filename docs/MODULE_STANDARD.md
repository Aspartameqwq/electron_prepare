# 模块接口与中断边界（P2）

当前实现遵循 [PLAN](../PLAN.md) §六/§十七；后续模块按阶段补充。

- 依赖方向：app → middleware → BSP → SysConfig/DriverLib；scheduler 无 BSP 依赖。
- 公共头不暴露 MCU 实例、生成宏或寄存器；诊断返回值副本，不返回内部任务控制块。
- 固件唯一 main 在 app/main.c；APP 经 app_dispatch 选择；tests/host 的 main 不进入固件。
- 时间单位 ms；uint32_t 约 49.7 天回绕，差值比较的有效区间严格小于 2^31 ms。
- scheduler 只用于主循环；回调有界。回调内仅 disable/诊断可用，init/register/enable 返回 BUSY；disable 对本轮未执行任务立即生效，递归调度直接返回。
- timebase_init 幂等，不重置运行中的 tick；tick 用 alignas(4) volatile uint32_t，单 ISR 写、主循环单字读。
- 只有 board/bsp/interrupts.c 定义真实 ISR；启动前清残留事件/使能 NVIC，正常 ISR 只读一次 CPU_INT.IIDX，ZERO 解码后调用只 tick++ 的 hook。
- IIDX 读即确认最高优先级事件，不再重复清位，避免丢失新事件。依据：[TI TRM SLAU846C §7.2，p.614](https://www.ti.com/lit/ug/slau846a/slau846a.pdf)；SDK 2.10 dl_timer.h 的 DL_Timer_getPendingInterrupt 直接读取 CPU_INT.IIDX。
- timebase_selfcheck_blocking 仅启动时调用：CPU 周期有限循环约 4ms，至少两个 tick 才成功；退出不依赖 tick，ISR 禁用。
- P2 console 为单消息静态缓冲，完整复制或拒绝；每轮最多 16 字节，FIFO 满立即返回。无堆分配/ISR 格式化，不是 P3 正式环形日志驱动。
- P2 链接栈为 2048B，大日志缓冲静态分配；栈高水位尚未实测。
