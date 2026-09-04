/**
 * @file timebase.h
 * @brief 毫秒时间源语义接口；仅依赖标准类型，换 MCU 时替换 BSP 实现。
 * @details board_init 后调用一次 init；主循环每轮读 now_ms。uint32_t 约 49.7 天回绕，
 * 时间差比较要求间隔 <2^31 ms（约 24.9 天）。中断中只能调用注明 isr_safe 的 hook。
 */
#ifndef TIMEBASE_H_
#define TIMEBASE_H_
#include <stdbool.h>
#include <stdint.h>
/** 启动时间源；重复调用幂等，不清零已运行的时间。仅主循环调用。 */
void timebase_init(void);
/** 返回启动后的毫秒数；自然对齐单字读取，isr_safe，无等待。 */
uint32_t timebase_now_ms(void);
/** ISR 唯一入口：只递增 tick；isr_safe，硬件事件由集中式 ISR 确认。 */
void timebase_on_period_irq(void);
/** 启动后自检；有限忙等约 4ms，至少收到两个 tick 才返回 true。
 * 中断失效也会有限返回 false；仅启动阶段主循环调用，禁止 ISR 调用。
 */
bool timebase_selfcheck_blocking(void);
#endif
