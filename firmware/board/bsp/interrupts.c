/**
 * @file interrupts.c
 * @brief 全工程真实 ISR 唯一定义点；依赖 DriverLib 和 timebase 的 isr_safe hook。
 * @details CPU_INT.IIDX 读即确认最高优先级事件（SLAU846C §7.2）；
 *          不再重复写 ICLR，避免把读取之后到达的新事件清掉。
 */
#include "ti_msp_dl_config.h"
#include "board/bsp/interrupts.h"
#include "board/bsp/timebase.h"
/** 仅在停止状态清启动残留，正常运行中的确认只在 ISR 读取 IIDX 时发生。 */
void interrupts_timebase_enable(void)
{
    DL_TimerG_clearInterruptStatus(SYS_TICK_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT);
    NVIC_ClearPendingIRQ(SYS_TICK_INST_INT_IRQN);
    NVIC_EnableIRQ(SYS_TICK_INST_INT_IRQN);
}
/** 硬件 ISR：唯一读取 IIDX；ZERO 解码后仅调用 tick++ hook，无等待/日志/调度。 */
void SYS_TICK_INST_IRQHandler(void)
{
    if (DL_TimerG_getPendingInterrupt(SYS_TICK_INST) == DL_TIMER_IIDX_ZERO) {
        timebase_on_period_irq();
    }
}
