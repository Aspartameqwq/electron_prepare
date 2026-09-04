/**
 * @file interrupts.h
 * @brief BSP 中断入口管理；仅供 BSP 使用，应用不需要知道 IRQ 号。
 * @details timebase_init 在启动计数器前调用 enable；硬件事件在 interrupts.c 唯一确认。
 */
#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_
/** 清理启动前遗留事件并使能时间源 IRQ；仅在计数器尚未启动时调用一次。 */
void interrupts_timebase_enable(void);
#endif
