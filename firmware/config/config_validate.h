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

_Static_assert(FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT > 0u,
               "FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT 必须为正");
_Static_assert(FRAME_TOTAL_TIMEOUT_MS_DEFAULT > FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT,
               "FRAME_TOTAL_TIMEOUT_MS_DEFAULT 必须大于字节间隔超时");
_Static_assert(FRAME_PAYLOAD_MAX > 0u && FRAME_PAYLOAD_MAX <= 128u,
               "FRAME_PAYLOAD_MAX 须在 (0, 128]");

#endif /* CONFIG_VALIDATE_H_ */
