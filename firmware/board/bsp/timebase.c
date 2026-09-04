/**
 * @file timebase.c
 * @brief TIMG12 毫秒时间源；依赖 SysConfig/DriverLib，使用方法见 timebase.h。
 * @details SysConfig 设置周期并使能 ZERO 事件，集中式 interrupts.c 负责事件确认。
 */
#include "ti_msp_dl_config.h"
#include "board/bsp/timebase.h"
#include "board/bsp/interrupts.h"
#include "project_config.h"
#include <stdalign.h>
/** 自然 4 字节对齐，单 ISR 写，主循环单字读；禁止主循环运行期间清零。 */
alignas(4) static volatile uint32_t g_tick_ms;
static bool g_started;
/** 显式启动；重复调用不扰动计数相位。 */
void timebase_init(void)
{
    if (g_started) { return; }
    interrupts_timebase_enable();
    g_started = true;
    DL_TimerG_startCounter(SYS_TICK_INST);
}
/** 原子读取毫秒快照，回绕由调用者用差值处理。 */
uint32_t timebase_now_ms(void) { return g_tick_ms; }
/** isr_safe：严格只增加 tick，不访问 IIDX 或清中断。 */
void timebase_on_period_irq(void) { ++g_tick_ms; }
/** 独立于 tick 的有限循环，避免中断未运行时自检自身永久等待。 */
bool timebase_selfcheck_blocking(void)
{
    const uint32_t begin_ms = timebase_now_ms();
    for (uint32_t i = 0u; i < TIMEBASE_SELFCHECK_WAIT_MS; ++i) {
        delay_cycles(CPUCLK_FREQ / 1000u);
    }
    return (uint32_t)(timebase_now_ms() - begin_ms) >= TIMEBASE_SELFCHECK_MIN_TICKS;
}
