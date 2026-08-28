/**
 * @file app_config.h
 * @brief 当前构建选择（PLAN.md §一：app_config.h 只管"当前 APP_ID / 链路类型 / 测试开关"）
 *
 * P1 只有单一 bringup 程序，暂无测试分发（app_dispatch 属 P2+）；APP_ID 预留枚举占位，
 * 非法值编译期报错（禁默认回退）。
 */
#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/** 应用程序标识：P1 仅 LED_BRINGUP 一个（后续阶段按 PLAN 增补）。 */
typedef enum {
    APP_ID_LED_BRINGUP = 1,   /* P1：PB22 LED 心跳 + UART0 启动日志 */
} app_id_t;

/** 当前选择的 APP（编译期固定；非法值在 config 校验处报错） */
#define APP_SELECTED_ID   APP_ID_LED_BRINGUP

#if APP_SELECTED_ID != APP_ID_LED_BRINGUP
#error "app_config.h: APP_SELECTED_ID 不是合法的 app_id_t 值"
#endif

/** UART1 链路类型（P3 起使用；P1 预留，保持显式而非缺省） */
typedef enum {
    UART1_LINK_LOOPBACK = 0,   /* 板内 TX/RX 短接回环（115200） */
    UART1_LINK_K230_DIRECT,    /* K230 直连（两端共同配置波特率） */
    UART1_LINK_HC04_BRIDGE,    /* HC-04 桥接（须读真实波特率，不得假定） */
} uart1_link_t;

#endif /* APP_CONFIG_H_ */
