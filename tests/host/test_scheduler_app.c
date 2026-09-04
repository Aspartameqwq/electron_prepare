/**
 * @file test_scheduler_app.c
 * @brief 编译真实板端 APP，替换 BSP 输出，验证 10min 成功/漏跑失败/结果冻结。
 * @details 不模拟 MCU 定时器；只验证应用验收逻辑，不能替代上板证据。
 */
#include "app/tests/test_scheduler.h"
#include "board/bsp/board.h"
#include "middleware/scheduler.h"
#include "project_config.h"
#include <stdio.h>
#include <string.h>
static char g_message[P2_CONSOLE_CAPACITY];
static unsigned g_failures;
#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); ++g_failures; } } while (0)
/** 测试替身：立即接收消息，仅保存文本，不模拟串口性能。 */
bool board_console_write(const char *text, size_t length)
{
    if (length >= sizeof(g_message)) { return false; }
    memcpy(g_message, text, length); g_message[length] = '\0'; return true;
}
/** 测试替身：心跳不影响逻辑时间。 */
void board_led_toggle(void) {}
/** 每轮严格采用与 main 相同的调度→APP 顺序。 */
static void step(uint32_t now_ms)
{
    scheduler_run_once(now_ms); test_scheduler_run_once(now_ms);
}
/** 正常与故意漏跑共享真实 APP，验证结果不能出现假成功。 */
static void simulate(bool inject_gap)
{
    const uint32_t start = UINT32_MAX - 5u;
    CHECK(scheduler_init() == SCHEDULER_OK);
    CHECK(test_scheduler_init());
    g_message[0] = '\0';
    step(start);
    for (uint32_t elapsed = 1u; elapsed <= P2_TEST_DURATION_MS; ++elapsed) {
        if (inject_gap && elapsed >= 100u && elapsed < 105u) { continue; }
        step(start + elapsed);
    }
    CHECK(strstr(g_message, inject_gap ? "RESULT_FAIL" : "RESULT_PASS") != NULL);
    CHECK(strstr(g_message, "elapsed_ms=600000") != NULL);
    if (!inject_gap) {
        CHECK(strstr(g_message, "count=600000/60000/30000") != NULL);
        CHECK(strstr(g_message, "missed=0/0/0") != NULL);
        CHECK(strstr(g_message, "order_errors=0 tick_errors=0") != NULL);
    }
    char frozen[P2_CONSOLE_CAPACITY];
    memcpy(frozen, g_message, sizeof(frozen));
    step(start + P2_TEST_DURATION_MS + P2_REPORT_PERIOD_MS);
    CHECK(strcmp(frozen, g_message) == 0);
    for (task_id_t id = 1u; id <= 3u; ++id) {
        scheduler_stats_t stats;
        CHECK(scheduler_get_stats(id, &stats) == SCHEDULER_OK && !stats.enabled);
    }
    test_scheduler_deinit();
}
/** Host 独立入口，不进入 CCS 构建。 */
int main(void)
{
    simulate(false); simulate(true);
    printf("scheduler app: %s (%u failures)\n", g_failures ? "FAILED" : "ALL PASSED", g_failures);
    return g_failures ? 1 : 0;
}
