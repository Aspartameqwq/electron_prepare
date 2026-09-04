/**
 * @file app_dispatch.c
 * @brief 唯一应用选择点；无需 CCS 排除文件或增加 main。
 * @details 依赖 app_config、BSP LED 与调度验收 APP。默认选择 P2。
 */
#include "app/app_config.h"
#include "app/app_interface.h"
#include "app/tests/test_scheduler.h"
#include "board/bsp/board.h"
#include "project_config.h"
#if APP_SELECTED_ID == APP_ID_LED_BRINGUP
static bool g_led_started;
static uint32_t g_led_due;
/** P1 心跳应用初始化，不注册合成任务。 */
static bool led_init(void) { g_led_started = false; return true; }
/** 使用毫秒快照维护心跳，不阻塞主循环。 */
static void led_run_once(uint32_t now_ms)
{
    if (!g_led_started) { g_led_due = now_ms; g_led_started = true; }
    if ((int32_t)(now_ms - g_led_due) >= 0) {
        board_led_toggle();
        g_led_due += ((now_ms - g_led_due) / P2_LED_HALF_PERIOD_MS + 1u) * P2_LED_HALF_PERIOD_MS;
    }
}
/** 清除应用相位；不修改时间源。 */
static void led_deinit(void) { g_led_started = false; }
#endif
/** 静态表仅包含当前选定实现，始终返回合法地址。 */
const app_ops_t *app_get_selected(void)
{
#if APP_SELECTED_ID == APP_ID_SCHEDULER_TEST
    static const app_ops_t ops = {test_scheduler_init, test_scheduler_run_once, test_scheduler_deinit};
#else
    static const app_ops_t ops = {led_init, led_run_once, led_deinit};
#endif
    return &ops;
}
