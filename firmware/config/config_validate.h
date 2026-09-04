/**
 * @file config_validate.h
 * @brief 编译期配置合法性检查（_Static_assert）
 *
 * 规则（见 PLAN.md §1）：
 *   - 所有断言由对应 *_CONFIG_READY 标志门控；本文件内为纯算法参数，无硬件依赖，故无条件断言。
 *   - READY=0 时对应参数宏可不定义，相应断言不参与编译。
 */
#ifndef CONFIG_VALIDATE_H_
#define CONFIG_VALIDATE_H_

#include "frame_codec.h"

_Static_assert(P2_TEST_DURATION_MS > 0u && P2_TEST_DURATION_MS < INT32_MAX,
               "P2 duration must fit signed time difference");
_Static_assert(P2_TASK_FAST_PERIOD_MS == 1u && P2_TASK_MEDIUM_PERIOD_MS == 10u &&
               P2_TASK_SLOW_PERIOD_MS == 20u, "P2 acceptance requires 1/10/20ms");
_Static_assert(P2_TEST_DURATION_MS % P2_TASK_SLOW_PERIOD_MS == 0u,
               "P2 duration must contain whole task periods");
_Static_assert(P2_REPORT_PERIOD_MS > 0u && P2_REPORT_PERIOD_MS < INT32_MAX &&
               P2_LED_HALF_PERIOD_MS > 0u && P2_LED_HALF_PERIOD_MS < INT32_MAX,
               "P2 service periods must fit signed time difference");
_Static_assert(P2_CONSOLE_CAPACITY >= 512u && P2_CONSOLE_BYTES_PER_SERVICE > 0u &&
               P2_CONSOLE_BYTES_PER_SERVICE <= P2_CONSOLE_CAPACITY, "invalid console budget");
_Static_assert(TIMEBASE_SELFCHECK_WAIT_MS > TIMEBASE_SELFCHECK_MIN_TICKS &&
               TIMEBASE_SELFCHECK_WAIT_MS <= 10u, "startup selfcheck must be bounded");

_Static_assert(FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT > 0u,
               "FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT 必须为正");
_Static_assert(FRAME_TOTAL_TIMEOUT_MS_DEFAULT > FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT,
               "FRAME_TOTAL_TIMEOUT_MS_DEFAULT 必须大于字节间隔超时");
_Static_assert(FRAME_PAYLOAD_MAX > 0u && FRAME_PAYLOAD_MAX <= 128u,
               "FRAME_PAYLOAD_MAX 须在 (0, 128]");

#endif /* CONFIG_VALIDATE_H_ */
