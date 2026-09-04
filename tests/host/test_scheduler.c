/**
 * @file test_scheduler.c
 * @brief 调度器真实 C 实现的 host 验收；无硬件或随机依赖。
 * @details 验证参数/容量、实际调用序、同一时间快照、迟到/漏跑、禁用、回绕及 10min 仿真。
 * 用 scripts/test_host.ps1 或其 -Sanitize 选项运行；失败返回非零。
 */
#include <stdio.h>
#include <string.h>
#include "scheduler.h"
static unsigned g_failures;
#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); ++g_failures; } } while (0)
static uint32_t g_count[3];
static uint32_t g_time[3];
static unsigned g_order[16];
static size_t g_order_count;
/** 回调计数独立于有限顺序日志，避免日志满误报调用丢失。 */
static void record(unsigned id, uint32_t now_ms)
{
    ++g_count[id]; g_time[id] = now_ms;
    if (g_order_count < 16u) { g_order[g_order_count++] = id; }
}
static void cb0(uint32_t now_ms) { record(0u, now_ms); }
static void cb1(uint32_t now_ms) { record(1u, now_ms); }
static void cb2(uint32_t now_ms) { record(2u, now_ms); }
/** 每例重建独立状态，检查 init 成功。 */
static void reset(void)
{
    CHECK(scheduler_init() == SCHEDULER_OK);
    memset(g_count, 0, sizeof(g_count)); memset(g_time, 0, sizeof(g_time)); g_order_count = 0u;
}
/** 通过公开接口取得副本；不依赖内部任务布局。 */
static scheduler_stats_t stats(task_id_t id)
{
    scheduler_stats_t s = {0};
    CHECK(scheduler_get_stats(id, &s) == SCHEDULER_OK); return s;
}
/** 覆盖空槽与保留 ID，避免 enable 非法空槽后出现零周期/空回调。 */
static void test_validation(void)
{
    reset();
    scheduler_stats_t s;
    CHECK(scheduler_enable(UINT8_MAX, 0u) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_disable(UINT8_MAX) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_get_stats(UINT8_MAX, &s) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_get_stats(0u, NULL) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_enable(0u, 0u) == SCHEDULER_ERR_NOT_FOUND);
    CHECK(scheduler_disable(0u) == SCHEDULER_ERR_NOT_FOUND);
    CHECK(scheduler_get_stats(0u, &s) == SCHEDULER_ERR_NOT_FOUND);
    CHECK(scheduler_register(UINT8_MAX, 1u, 0u, cb0) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_register(0u, 0u, 0u, cb0) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_register(0u, 0x80000000u, 0u, cb0) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_register(0u, UINT32_MAX, 0u, cb0) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_register(0u, 1u, 0u, NULL) == SCHEDULER_ERR_INVALID_ARG);
    CHECK(scheduler_register(0u, INT32_MAX, 0u, cb0) == SCHEDULER_OK);
    CHECK(scheduler_enable(0u, 0u) == SCHEDULER_OK);
    scheduler_run_once(INT32_MAX - 1u); CHECK(g_count[0] == 0u);
    scheduler_run_once(INT32_MAX); CHECK(g_count[0] == 1u);
    CHECK(scheduler_register(0u, 1u, 0u, cb0) == SCHEDULER_ERR_DUP_ID);
    for (task_id_t i = 1u; i < SCHEDULER_MAX_TASKS; ++i) {
        CHECK(scheduler_register(i, 10u, 0u, cb0) == SCHEDULER_OK);
    }
    CHECK(scheduler_task_count() == SCHEDULER_MAX_TASKS);
    CHECK(scheduler_register(100u, 10u, 0u, cb0) == SCHEDULER_ERR_FULL);
}
/** 故意反序注册，验证顺序本身及快照值，不能只验证调用次数。 */
static void test_order_snapshot(void)
{
    reset();
    CHECK(scheduler_register(30u, 10u, 2u, cb0) == SCHEDULER_OK);
    CHECK(scheduler_register(20u, 10u, 1u, cb1) == SCHEDULER_OK);
    CHECK(scheduler_register(10u, 10u, 1u, cb2) == SCHEDULER_OK);
    CHECK(scheduler_enable(30u, 0u) == SCHEDULER_OK);
    CHECK(scheduler_enable(20u, 0u) == SCHEDULER_OK);
    CHECK(scheduler_enable(10u, 0u) == SCHEDULER_OK);
    for (uint32_t now = 10u; now <= 30u; now += 10u) {
        g_order_count = 0u;
        scheduler_run_once(now);
        CHECK(g_order_count == 3u);
        CHECK(g_order[0] == 2u && g_order[1] == 1u && g_order[2] == 0u);
        CHECK(g_time[0] == now && g_time[1] == now && g_time[2] == now);
        scheduler_run_once(now); CHECK(g_order_count == 3u);
    }
}
/** 精确期限与漏跑边界，不补跑、不漂移；停用任务不增加统计。 */
static void test_deadlines(void)
{
    reset();
    CHECK(scheduler_register(0u, 10u, 0u, cb0) == SCHEDULER_OK);
    scheduler_run_once(1000u); CHECK(g_count[0] == 0u);
    CHECK(scheduler_enable(0u, 100u) == SCHEDULER_OK);
    scheduler_run_once(100u); scheduler_run_once(109u); CHECK(g_count[0] == 0u);
    scheduler_run_once(110u); CHECK(g_count[0] == 1u);
    scheduler_run_once(129u); CHECK(stats(0u).missed_count == 0u);
    CHECK(stats(0u).last_lateness_ms == 9u);
    scheduler_run_once(140u); CHECK(g_count[0] == 3u);
    CHECK(stats(0u).missed_count == 1u && stats(0u).next_due_ms == 150u);
    scheduler_run_once(185u); CHECK(g_count[0] == 4u);
    CHECK(stats(0u).missed_count == 4u && stats(0u).next_due_ms == 190u);
    CHECK(stats(0u).max_lateness_ms == 35u);
    CHECK(scheduler_disable(0u) == SCHEDULER_OK);
    scheduler_run_once(10000u); CHECK(g_count[0] == 4u && stats(0u).missed_count == 4u);
    CHECK(scheduler_enable(0u, 10000u) == SCHEDULER_OK);
    scheduler_run_once(10009u); CHECK(g_count[0] == 4u);
    scheduler_run_once(10010u); CHECK(g_count[0] == 5u && stats(0u).missed_count == 4u);
}
/** 高优先级回调停止本轮低优先级任务，修改表/递归调用被拒绝。 */
static void cancel_callback(uint32_t now_ms)
{
    record(0u, now_ms);
    CHECK(scheduler_disable(1u) == SCHEDULER_OK);
    CHECK(scheduler_disable(0u) == SCHEDULER_OK);
    CHECK(scheduler_init() == SCHEDULER_ERR_BUSY);
    CHECK(scheduler_register(2u, 1u, 0u, cb2) == SCHEDULER_ERR_BUSY);
    CHECK(scheduler_enable(1u, now_ms) == SCHEDULER_ERR_BUSY);
    scheduler_run_once(now_ms);
}
static void test_callback_control(void)
{
    reset();
    CHECK(scheduler_register(1u, 1u, 1u, cb1) == SCHEDULER_OK);
    CHECK(scheduler_register(0u, 1u, 0u, cancel_callback) == SCHEDULER_OK);
    CHECK(scheduler_enable(1u, 0u) == SCHEDULER_OK);
    CHECK(scheduler_enable(0u, 0u) == SCHEDULER_OK);
    scheduler_run_once(4u);
    CHECK(g_count[0] == 1u && g_count[1] == 0u);
    CHECK(stats(1u).missed_count == 0u);
    CHECK(!stats(0u).enabled && !stats(1u).enabled);
}
/** 10min 逐 ms 仿真跨回绕；运行时间是逻辑时间，不代替板端 10min。 */
static void test_ten_minutes_wrap(void)
{
    reset();
    const uint32_t start = UINT32_MAX - 25u;
    const uint32_t periods[3] = {1u, 10u, 20u};
    scheduler_task_fn_t callbacks[3] = {cb0, cb1, cb2};
    for (task_id_t i = 0u; i < 3u; ++i) {
        CHECK(scheduler_register(i, periods[i], i, callbacks[i]) == SCHEDULER_OK);
        CHECK(scheduler_enable(i, start) == SCHEDULER_OK);
    }
    for (uint32_t elapsed = 0u; elapsed <= 600000u; ++elapsed) { scheduler_run_once(start + elapsed); }
    CHECK(g_count[0] == 600000u && g_count[1] == 60000u && g_count[2] == 30000u);
    for (task_id_t i = 0u; i < 3u; ++i) {
        CHECK(stats(i).missed_count == 0u && stats(i).max_lateness_ms == 0u);
    }
    reset();
    CHECK(scheduler_register(0u, 10u, 0u, cb0) == SCHEDULER_OK);
    CHECK(scheduler_enable(0u, UINT32_MAX - 5u) == SCHEDULER_OK);
    scheduler_run_once(3u); CHECK(g_count[0] == 0u);
    scheduler_run_once(4u); CHECK(g_count[0] == 1u);
    scheduler_run_once(34u);
    CHECK(g_count[0] == 2u && stats(0u).missed_count == 2u && stats(0u).next_due_ms == 44u);
}
/** Host 独立入口；不进入 CCS 固件构建。 */
int main(void)
{
    test_validation(); test_order_snapshot(); test_deadlines();
    test_callback_control(); test_ten_minutes_wrap();
    printf("scheduler: %s (%u failures; 600000ms wrap simulation)\n", g_failures ? "FAILED" : "ALL PASSED", g_failures);
    return g_failures ? 1 : 0;
}
