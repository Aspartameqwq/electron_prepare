/**
 * @file test_scheduler.c
 * @brief P2 的 1/10/20ms、10min 板端验收 APP，无硬件寄存器依赖。
 * @details 通过 app_dispatch 使用。回调只记数/验证顺序；日志在回调外有界格式化，
 *          拷贝至 BSP 后逐轮发 FIFO。1ms 合成任务仅 P2 使用，P10 不注册。
 */
#include "app/tests/test_scheduler.h"
#include "middleware/scheduler.h"
#include "board/bsp/board.h"
#include "project_config.h"
#include <stdio.h>
#include <string.h>
/** 测试 ID：同点先 slow，再 medium，再 fast；故意与注册顺序相反。 */
enum { TASK_SLOW = 1, TASK_MEDIUM = 2, TASK_FAST = 3, TEST_TASK_COUNT = 3 };
static const task_id_t g_ids[TEST_TASK_COUNT] = {TASK_FAST, TASK_MEDIUM, TASK_SLOW};
static const uint32_t g_periods[TEST_TASK_COUNT] = {
    P2_TASK_FAST_PERIOD_MS, P2_TASK_MEDIUM_PERIOD_MS, P2_TASK_SLOW_PERIOD_MS
};
static uint32_t g_counts[TEST_TASK_COUNT];
static scheduler_stats_t g_stats[TEST_TASK_COUNT];
static uint32_t g_start_ms, g_last_ms, g_elapsed_ms, g_report_due_ms, g_led_due_ms;
static uint32_t g_order_ms, g_order_errors, g_tick_errors, g_report_drops;
static task_id_t g_last_order_id;
static bool g_started, g_done, g_order_seen, g_pass;
/** 单主循环使用静态格式化区，避免 512B 消息占用调用栈。 */
static char g_message[P2_CONSOLE_CAPACITY];

/** 记录真实调用顺序；同一快照应按 ID 1/2/3 执行，反序即计错。 */
static void record_task(size_t index, uint32_t now_ms)
{
    const task_id_t id = g_ids[index];
    if (g_order_seen && g_order_ms == now_ms && id <= g_last_order_id) { ++g_order_errors; }
    if (!g_order_seen || g_order_ms != now_ms) { g_order_ms = now_ms; g_order_seen = true; }
    g_last_order_id = id;
    ++g_counts[index];
}
/** 1ms 压力任务，仅计数。 */
static void task_fast(uint32_t now_ms) { record_task(0u, now_ms); }
/** 10ms 合成任务，仅计数。 */
static void task_medium(uint32_t now_ms) { record_task(1u, now_ms); }
/** 20ms 合成任务，优先于其他合成任务。 */
static void task_slow(uint32_t now_ms) { record_task(2u, now_ms); }
/** 清空 APP 自身统计并注册，失败时让入口保持任务禁用。 */
bool test_scheduler_init(void)
{
    memset(g_counts, 0, sizeof(g_counts));
    memset(g_stats, 0, sizeof(g_stats));
    g_started = false; g_done = false; g_order_seen = false; g_pass = false;
    g_order_errors = 0u; g_tick_errors = 0u; g_report_drops = 0u;
    return scheduler_register(TASK_FAST, P2_TASK_FAST_PERIOD_MS, 1u, task_fast) == SCHEDULER_OK &&
           scheduler_register(TASK_MEDIUM, P2_TASK_MEDIUM_PERIOD_MS, 1u, task_medium) == SCHEDULER_OK &&
           scheduler_register(TASK_SLOW, P2_TASK_SLOW_PERIOD_MS, 0u, task_slow) == SCHEDULER_OK;
}
/** 停用全部测试任务；已停用时仍成功，注册信息留作诊断。 */
void test_scheduler_deinit(void)
{
    for (size_t i = 0u; i < TEST_TASK_COUNT; ++i) { (void)scheduler_disable(g_ids[i]); }
}
/** 比较固定 10min 内理论次数，误差最多一次；统计失败不能默认为通过。 */
static void finish_test(void)
{
    g_pass = g_tick_errors == 0u && g_order_errors == 0u &&
             g_elapsed_ms - P2_TEST_DURATION_MS <= P2_TASK_FAST_PERIOD_MS;
    for (size_t i = 0u; i < TEST_TASK_COUNT; ++i) {
        const uint32_t expected = P2_TEST_DURATION_MS / g_periods[i];
        const uint32_t error = g_counts[i] > expected ? g_counts[i] - expected : expected - g_counts[i];
        if (error > 1u || g_stats[i].missed_count != 0u) { g_pass = false; }
    }
    test_scheduler_deinit();
    g_done = true;
}
/** 有界构造消息，忙/长度异常计丢弃，下个报告点重试；结果统计完成后冻结。 */
static void report(void)
{
    const int n = snprintf(g_message, sizeof(g_message),
        "P2 %s elapsed_ms=%lu count=%lu/%lu/%lu missed=%lu/%lu/%lu maxlate_ms=%lu/%lu/%lu order_errors=%lu tick_errors=%lu report_drops=%lu\r\n",
        g_done ? (g_pass ? "RESULT_PASS" : "RESULT_FAIL") : "RUNNING",
        (unsigned long)g_elapsed_ms,
        (unsigned long)g_counts[0], (unsigned long)g_counts[1], (unsigned long)g_counts[2],
        (unsigned long)g_stats[0].missed_count, (unsigned long)g_stats[1].missed_count, (unsigned long)g_stats[2].missed_count,
        (unsigned long)g_stats[0].max_lateness_ms, (unsigned long)g_stats[1].max_lateness_ms, (unsigned long)g_stats[2].max_lateness_ms,
        (unsigned long)g_order_errors, (unsigned long)g_tick_errors, (unsigned long)g_report_drops);
    if (n <= 0 || (size_t)n >= sizeof(g_message) || !board_console_write(g_message, (size_t)n)) { ++g_report_drops; }
}
/** 单快照驱动：最终回调执行后冻结结果，继续心跳并定期重发，方便串口晚接入。 */
void test_scheduler_run_once(uint32_t now_ms)
{
    if (!g_started) {
        g_start_ms = now_ms; g_last_ms = now_ms; g_led_due_ms = now_ms;
        g_report_due_ms = now_ms + P2_REPORT_PERIOD_MS;
        g_started = true;
        for (size_t i = 0u; i < TEST_TASK_COUNT; ++i) {
            if (scheduler_enable(g_ids[i], now_ms) != SCHEDULER_OK) {
                ++g_tick_errors; test_scheduler_deinit(); g_done = true; g_pass = false;
                return;
            }
        }
    }
    if (!g_done) {
        if ((int32_t)(now_ms - g_last_ms) < 0) { ++g_tick_errors; }
        g_last_ms = now_ms;
        g_elapsed_ms = now_ms - g_start_ms;
        for (size_t i = 0u; i < TEST_TASK_COUNT; ++i) {
            if (scheduler_get_stats(g_ids[i], &g_stats[i]) != SCHEDULER_OK) { ++g_tick_errors; }
        }
        if (g_elapsed_ms >= P2_TEST_DURATION_MS) { finish_test(); g_report_due_ms = now_ms; }
    }
    if ((int32_t)(now_ms - g_led_due_ms) >= 0) {
        board_led_toggle();
        g_led_due_ms += ((now_ms - g_led_due_ms) / P2_LED_HALF_PERIOD_MS + 1u) * P2_LED_HALF_PERIOD_MS;
    }
    if ((int32_t)(now_ms - g_report_due_ms) >= 0) {
        report();
        g_report_due_ms += ((now_ms - g_report_due_ms) / P2_REPORT_PERIOD_MS + 1u) * P2_REPORT_PERIOD_MS;
    }
}
