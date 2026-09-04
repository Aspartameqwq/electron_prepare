/**
 * @file app_config.h
 * @brief 当前 APP 构建选择；ID 使用数字宏供预处理器检查，非法值直接编译失败。
 * @details P2 选择 scheduler 验收；P1 LED 应用通过同一分发器保留。
 */
#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_
#include <stdint.h>
typedef uint8_t app_id_t;
/** DESIGN_DEFAULT：构建 ID，无单位。 */
#define APP_ID_LED_BRINGUP 1u
#define APP_ID_SCHEDULER_TEST 2u
#ifndef APP_SELECTED_ID
#define APP_SELECTED_ID APP_ID_SCHEDULER_TEST
#endif
#if APP_SELECTED_ID != APP_ID_LED_BRINGUP && APP_SELECTED_ID != APP_ID_SCHEDULER_TEST
#error "APP_SELECTED_ID is invalid"
#endif
/** UART1 链路枚举；P3 才选择实际链路，当前不启用外设。 */
typedef enum {
    UART1_LINK_LOOPBACK = 0, UART1_LINK_K230_DIRECT, UART1_LINK_HC04_BRIDGE
} uart1_link_t;
#endif
