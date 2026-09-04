/**
 * @file scheduler.c
 * @brief 纯软件调度器；内部静态表由主循环拥有，无 SDK/堆分配依赖。
 * @details 用法见 scheduler.h；回调中只允许 disable/诊断，避免改变本轮候选集。
 */
#include "scheduler.h"
/** 内部任务槽；有效性与 ID 分离，保留 ID 不会误匹配空槽。 */
typedef struct {
    bool registered;
    task_id_t id;
    task_priority_t priority;
    uint32_t period_ms;
    scheduler_task_fn_t callback;
    scheduler_stats_t stats;
} scheduler_task_t;
static scheduler_task_t g_tasks[SCHEDULER_MAX_TASKS];
static bool g_running;
_Static_assert(SCHEDULER_MAX_TASKS > 0u && SCHEDULER_MAX_TASKS <= 16u, "capacity must be 1..16");
/** 查已注册槽；未找到返回 NULL。 */
static scheduler_task_t *find_task(task_id_t id)
{
    for (size_t i = 0u; i < SCHEDULER_MAX_TASKS; ++i) {
        if (g_tasks[i].registered && g_tasks[i].id == id) { return &g_tasks[i]; }
    }
    return NULL;
}
/** 清空状态；遍历回调期间不能销毁表。 */
scheduler_status_t scheduler_init(void)
{
    if (g_running) { return SCHEDULER_ERR_BUSY; }
    for (size_t i = 0u; i < SCHEDULER_MAX_TASKS; ++i) { g_tasks[i] = (scheduler_task_t){0}; }
    return SCHEDULER_OK;
}
/** 校验并占用首个空槽；失败不修改任务表。 */
scheduler_status_t scheduler_register(task_id_t id, uint32_t period_ms,
                                      task_priority_t priority, scheduler_task_fn_t callback)
{
    if (g_running) { return SCHEDULER_ERR_BUSY; }
    if (id == SCHEDULER_INVALID_TASK_ID || period_ms == 0u ||
        period_ms > (uint32_t)SCHEDULER_MAX_PERIOD_MS || callback == NULL) {
        return SCHEDULER_ERR_INVALID_ARG;
    }
    if (find_task(id) != NULL) { return SCHEDULER_ERR_DUP_ID; }
    for (size_t i = 0u; i < SCHEDULER_MAX_TASKS; ++i) {
        if (!g_tasks[i].registered) {
            g_tasks[i] = (scheduler_task_t){
                .registered = true, .id = id, .priority = priority,
                .period_ms = period_ms, .callback = callback
            };
            return SCHEDULER_OK;
        }
    }
    return SCHEDULER_ERR_FULL;
}
/** 从下一周期起算；历史统计保留。 */
scheduler_status_t scheduler_enable(task_id_t id, uint32_t now_ms)
{
    if (g_running) { return SCHEDULER_ERR_BUSY; }
    if (id == SCHEDULER_INVALID_TASK_ID) { return SCHEDULER_ERR_INVALID_ARG; }
    scheduler_task_t *t = find_task(id);
    if (t == NULL) { return SCHEDULER_ERR_NOT_FOUND; }
    t->stats.next_due_ms = now_ms + t->period_ms;
    t->stats.enabled = true;
    return SCHEDULER_OK;
}
/** 立即停用；本轮尚未执行的任务不会被调用。 */
scheduler_status_t scheduler_disable(task_id_t id)
{
    if (id == SCHEDULER_INVALID_TASK_ID) { return SCHEDULER_ERR_INVALID_ARG; }
    scheduler_task_t *t = find_task(id);
    if (t == NULL) { return SCHEDULER_ERR_NOT_FOUND; }
    t->stats.enabled = false;
    return SCHEDULER_OK;
}
/** 固定容量选择遍历，按 (priority,id) 执行；每任务至多一次。 */
void scheduler_run_once(uint32_t now_ms)
{
    if (g_running) { return; }
    g_running = true;
    bool executed[SCHEDULER_MAX_TASKS] = {false};
    for (size_t pass = 0u; pass < SCHEDULER_MAX_TASKS; ++pass) {
        size_t best = SIZE_MAX;
        for (size_t i = 0u; i < SCHEDULER_MAX_TASKS; ++i) {
            const scheduler_task_t *t = &g_tasks[i];
            if (!t->registered || !t->stats.enabled || executed[i] ||
                (int32_t)(now_ms - t->stats.next_due_ms) < 0) { continue; }
            if (best == SIZE_MAX || t->priority < g_tasks[best].priority ||
                (t->priority == g_tasks[best].priority && t->id < g_tasks[best].id)) { best = i; }
        }
        if (best == SIZE_MAX) { break; }
        scheduler_task_t *t = &g_tasks[best];
        const uint32_t late = now_ms - t->stats.next_due_ms;
        const uint32_t missed = late / t->period_ms;
        t->stats.missed_count += missed;
        t->stats.last_lateness_ms = late;
        if (late > t->stats.max_lateness_ms) { t->stats.max_lateness_ms = late; }
        /* 模加法允许跨 UINT32_MAX；保持原相位而不是 now+period，避免累计漂移。 */
        t->stats.next_due_ms += (missed + 1u) * t->period_ms;
        executed[best] = true;
        t->callback(now_ms);
    }
    g_running = false;
}
/** 复制诊断，不返回内部可变地址。 */
scheduler_status_t scheduler_get_stats(task_id_t id, scheduler_stats_t *out)
{
    if (id == SCHEDULER_INVALID_TASK_ID || out == NULL) { return SCHEDULER_ERR_INVALID_ARG; }
    const scheduler_task_t *t = find_task(id);
    if (t == NULL) { return SCHEDULER_ERR_NOT_FOUND; }
    *out = t->stats;
    return SCHEDULER_OK;
}
/** 返回注册数，包含停用任务。 */
size_t scheduler_task_count(void)
{
    size_t n = 0u;
    for (size_t i = 0u; i < SCHEDULER_MAX_TASKS; ++i) { if (g_tasks[i].registered) { ++n; } }
    return n;
}
