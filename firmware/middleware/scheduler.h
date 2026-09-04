/**
 * @file scheduler.h
 * @brief 纯 C11 协作调度器：静态容量、确定优先级、回绕安全、不补跑。
 * @details 无硬件依赖。启动时 init/register/enable；主循环每轮取一次时间调用 run_once。
 * 所有 API 仅限主循环。时间前进和服务间隔必须小于 2^31 ms（约 24.9 天）。
 */
#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/** DESIGN_DEFAULT：静态任务容量，范围 1..16。 */
#define SCHEDULER_MAX_TASKS 16u
/** CALCULATED：回绕安全比较的最大周期，单位 ms。 */
#define SCHEDULER_MAX_PERIOD_MS INT32_MAX
/** DESIGN_DEFAULT：保留 ID，合法任务 ID 为 0..254。 */
#define SCHEDULER_INVALID_TASK_ID UINT8_MAX
typedef uint8_t task_id_t;
/** 数值越小越优先；同优先级按 task_id 升序。 */
typedef uint8_t task_priority_t;
/** 参数为本轮时间快照；必须有界，不得等待或递归调度。 */
typedef void (*scheduler_task_fn_t)(uint32_t now_ms);
typedef enum {
    SCHEDULER_OK = 0, SCHEDULER_ERR_FULL, SCHEDULER_ERR_DUP_ID,
    SCHEDULER_ERR_INVALID_ARG, SCHEDULER_ERR_NOT_FOUND, SCHEDULER_ERR_BUSY
} scheduler_status_t;
/** 诊断值副本，不暴露内部表和回调；计数按 uint32_t 模回绕。 */
typedef struct {
    uint32_t next_due_ms;       /**< 下次到期时刻，ms。 */
    uint32_t missed_count;      /**< 累计漏跑周期，重新使能不清零。 */
    uint32_t last_lateness_ms;  /**< 最近实际执行的迟到量，ms。 */
    uint32_t max_lateness_ms;   /**< 注册以来最大迟到量，ms。 */
    bool enabled;
} scheduler_stats_t;
/** 清空表，可重复调用；回调执行中返回 BUSY，不改变状态。 */
scheduler_status_t scheduler_init(void);
/** 注册未启用任务；period_ms 范围 1..INT32_MAX，callback 不得为空。
 * @return OK / INVALID_ARG / DUP_ID / FULL；回调中返回 BUSY。
 */
scheduler_status_t scheduler_register(task_id_t id, uint32_t period_ms,
                                      task_priority_t priority, scheduler_task_fn_t callback);
/** 从 now_ms+period 开始；重复调用重定相位、保留统计。回调中返回 BUSY。
 * @return 保留 ID 返回 INVALID_ARG，未知 ID 返回 NOT_FOUND，否则 OK。
 */
scheduler_status_t scheduler_enable(task_id_t id, uint32_t now_ms);
/** 立即停用并保留统计；允许回调停用自己或本轮尚未执行的任务。
 * @return 保留 ID 返回 INVALID_ARG，未知 ID 返回 NOT_FOUND，否则 OK。
 */
scheduler_status_t scheduler_disable(task_id_t id);
/** 每轮每个到期任务最多执行一次；原相位推进，漏跑只计数。
 * 所有回调使用同一快照；递归调用直接返回，不重复执行。
 */
void scheduler_run_once(uint32_t now_ms);
/** 复制统计；out=NULL/保留 ID 返回 INVALID_ARG，未知 ID 返回 NOT_FOUND。 */
scheduler_status_t scheduler_get_stats(task_id_t id, scheduler_stats_t *out);
/** 返回已注册任务数，含停用任务。 */
size_t scheduler_task_count(void);
#endif
