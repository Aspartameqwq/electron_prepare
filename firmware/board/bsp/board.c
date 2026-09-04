/**
 * @file board.c
 * @brief P2 板级基础功能；硬件操作集中在此，应用只见语义接口。
 * @details 依赖 SysConfig、DriverLib、project_config。单条静态诊断消息无堆分配，
 *          轮询 FIFO 有字节预算；不是 P3 的正式 UART0 驱动。
 */
#include "ti_msp_dl_config.h"
#include "board/bsp/board.h"
#include "project_config.h"
#include <stdio.h>
#include <string.h>
static char g_tx[P2_CONSOLE_CAPACITY];
static size_t g_tx_length;
static size_t g_tx_offset;
static uint32_t g_reset_reason;
/** 初始化唯一 SysConfig，立即保存读清除的复位原因，不在应用暴露寄存器。 */
void board_init(void)
{
    SYSCFG_DL_init();
    g_reset_reason = (uint32_t)DL_SYSCTL_getResetCause();
}
/** 心跳不等待。 */
void board_led_toggle(void) { DL_GPIO_togglePins(LED_PORT, LED_PIN_PIN); }
/** 查询软件消息槽，不等待硬件移位寄存器。 */
bool board_console_idle(void) { return g_tx_offset == g_tx_length; }
/** 全消息复制或拒绝；主循环唯一生产者和消费者，无 ISR 并发。 */
bool board_console_write(const char *text, size_t length)
{
    if (text == NULL || length == 0u || length > sizeof(g_tx) || !board_console_idle()) {
        return false;
    }
    memcpy(g_tx, text, length);
    g_tx_length = length;
    g_tx_offset = 0u;
    return true;
}
/** 有限轮询；硬件 FIFO 无空间立即返回，因此不会拖延合成任务。 */
void board_console_service(void)
{
    for (size_t n = 0u; n < P2_CONSOLE_BYTES_PER_SERVICE && g_tx_offset < g_tx_length; ++n) {
        if (DL_UART_Main_isTXFIFOFull(DEBUG_UART_INST)) { break; }
        DL_UART_Main_transmitData(DEBUG_UART_INST, (uint8_t)g_tx[g_tx_offset++]);
    }
}
/** 启动时格式化一次；原始复位码留给验证记录解释。 */
bool board_print_banner(uint32_t app_id)
{
    char message[192];
    const int n = snprintf(message, sizeof(message),
        "\r\nMSPM0 control-lib v0.2.0 app_id=%lu\r\nCPUCLK=%lu Hz reset_code=%lu\r\nactuators=disabled\r\n",
        (unsigned long)app_id, (unsigned long)CPUCLK_FREQ, (unsigned long)g_reset_reason);
    return n > 0 && (size_t)n < sizeof(message) && board_console_write(message, (size_t)n);
}
