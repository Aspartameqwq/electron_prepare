/**
 * @file board.h
 * @brief 板级启动、心跳和 P2 诊断接口；公共头无 MCU 宏。
 * @details main 调 board_init 一次；每轮 console_service 推进已复制的消息。
 *          P2 使用单消息轮询发送，P3 再接入正式 UART0 中断环形队列。
 */
#ifndef BOARD_H_
#define BOARD_H_
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/** 初始化板级外设，可且仅可在启动时调用一次；执行器资源不启用。 */
void board_init(void);
/** 翻转心跳 LED，无等待，仅主循环调用。 */
void board_led_toggle(void);
/** 复制一条消息，忙/空指针/过长返回 false 且不部分入队；长度不含终止符。 */
bool board_console_write(const char *text, size_t length);
/** 每轮最多发送配置预算字节；FIFO 满立即返回，不等 UART。仅主循环调用。 */
void board_console_service(void);
/** 队列可接收下一条消息时返回 true；不保证最后字节已移出硬件 FIFO。 */
bool board_console_idle(void);
/** 构造并入队启动信息，app_id 为调用者的构建选择；忙时返回 false。 */
bool board_print_banner(uint32_t app_id);
#endif
