/**
 * @file test_scheduler.h
 * @brief P2 板端合成任务验收；经 app_dispatch 调用，不定义 main。
 * @details 依赖纯软件 scheduler、BSP 心跳/诊断与 project_config；无执行器依赖。
 */
#ifndef APP_TEST_SCHEDULER_H_
#define APP_TEST_SCHEDULER_H_
#include <stdbool.h>
#include <stdint.h>
/** 注册 1/10/20ms 合成任务；注册失败返回 false，不启用任何任务。 */
bool test_scheduler_init(void);
/** 首轮统一使能；运行 10min 后停用任务并周期输出冻结统计，非阻塞。 */
void test_scheduler_run_once(uint32_t now_ms);
/** 停用本 APP 注册的任务，可重复调用；不清零全局时间。 */
void test_scheduler_deinit(void);
#endif
