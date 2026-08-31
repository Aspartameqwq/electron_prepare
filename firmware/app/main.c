/**
 * @file main.c
 * @brief P1 最小工程唯一入口（PLAN.md §四：全工程唯一 main()）
 *
 * 安全启动序（PLAN.md P1 验收要求）：
 *   1. SYSCFG_DL_init()            —— SysConfig 生成的外设初始化（LED/UART0）
 *   2. 执行器保持禁用              —— P1 无电机/舵机驱动，天然禁用（无 PWM 定时器实例）
 *   3. 初始化调试 UART0            —— 已由 SYSCFG_DL_init 完成（本阶段直接阻塞发送）
 *   4. 输出启动信息                —— 固件版本 / 时钟 / APP_ID / 复位原因
 *   5. 启动 LED 心跳               —— PB22 1Hz 闪烁
 *
 * P1 阶段特性（过渡设计，P2 替换）：
 *   - delay 用 delay_cycles 忙等（P1 无 1ms 节拍；P2 引入 TIMG12 tick + 调度器后替换）；
 *   - UART0 发送用 DL_UART_transmitDataBlocking（阻塞 TX，P1 验收允许；P3 换非阻塞队列）；
 *   - 无 ISR 注册（集中式 interrupts.c 属 P2/P3）。
 *
 * 上板验收（P1 基础必做）：冷启动 ×3 均输出一次完整启动信息 + LED 1Hz 闪。
 */
#include "ti_msp_dl_config.h"
#include <stdint.h>

#include "app_config.h"

/* ---- 固件身份（P1 手工版本号；P10 起由构建脚本注入 git commit） ---- */
#define FW_VERSION_STR     "0.1.0"

/** 复位原因 → 文本（RSTCAUSE 枚举见 dl_sysctl_mspm0g1x0x_g3x0x.h @ref DL_SYSCTL_RESET_CAUSE） */
static const char *reset_reason_str(DL_SYSCTL_RESET_CAUSE cause)
{
    switch (cause) {
    case DL_SYSCTL_RESET_CAUSE_NO_RESET:               return "NO_RESET";
    case DL_SYSCTL_RESET_CAUSE_POR_HW_FAILURE:         return "POR_HW_FAIL";
    case DL_SYSCTL_RESET_CAUSE_POR_EXTERNAL_NRST:      return "POR_NRST";
    case DL_SYSCTL_RESET_CAUSE_POR_SW_TRIGGERED:       return "POR_SW";
    case DL_SYSCTL_RESET_CAUSE_BOR_SUPPLY_FAILURE:     return "BOR_SUPPLY";
    case DL_SYSCTL_RESET_CAUSE_BOR_WAKE_FROM_SHUTDOWN: return "BOR_WAKE";
    case DL_SYSCTL_RESET_CAUSE_BOOTRST_NON_PMU_PARITY_FAULT: return "BOOT_PARITY";
    case DL_SYSCTL_RESET_CAUSE_BOOTRST_CLOCK_FAULT:    return "BOOT_CLKFAIL";
    case DL_SYSCTL_RESET_CAUSE_BOOTRST_SW_TRIGGERED:   return "BOOT_SW";
    case DL_SYSCTL_RESET_CAUSE_BOOTRST_EXTERNAL_NRST:  return "BOOT_NRST";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_BSL_EXIT:        return "SYS_BSL_EXIT";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_BSL_ENTRY:       return "SYS_BSL_ENTRY";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_WWDT0_VIOLATION: return "SYS_WWDT0";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_WWDT1_VIOLATION: return "SYS_WWDT1";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_FLASH_ECC_ERROR: return "SYS_FLASH_ECC";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_CPU_LOCKUP_VIOLATION: return "SYS_CPULOCK";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_DEBUG_TRIGGERED: return "SYS_DEBUG";
    case DL_SYSCTL_RESET_CAUSE_SYSRST_SW_TRIGGERED:    return "SYS_SW";
    case DL_SYSCTL_RESET_CAUSE_CPURST_DEBUG_TRIGGERED: return "CPU_DEBUG";
    case DL_SYSCTL_RESET_CAUSE_CPURST_SW_TRIGGERED:    return "CPU_SW";
    default:                                           return "UNKNOWN";
    }
}

/** UART0 阻塞发送字符串（P1 过渡：DL_UART_transmitDataBlocking 内部等 BUSY；P3 换非阻塞队列） */
static void uart0_print(const char *s)
{
    while (*s != '\0') {
        DL_UART_transmitDataBlocking(DEBUG_UART_INST, (uint8_t)*s++);
    }
}

/** UART0 阻塞发送无符号整数（十进制；小工具，避免拖入 printf 库） */
static void uart0_print_u32(uint32_t v)
{
    char buf[11];                          /* 4294967295 + '\0' 最多 11 字节 */
    int i = (int)sizeof(buf) - 1;
    buf[i] = '\0';
    do {
        buf[--i] = (char)('0' + (v % 10u));
        v /= 10u;
    } while (v != 0u);
    uart0_print(&buf[i]);
}

/**
 * @brief delay_cycles 忙等延时（P1 过渡方案；P2 换 TIMG12 1ms 节拍）。
 * @param ms 延时毫秒数。按 1ms 逐次调用 delay_cycles（其参数有上限）。
 * @note cycles_per_ms 由 CPUCLK_FREQ 生成宏自动计算，随当前时钟基线自适应（当前 80MHz 基线 = 80000），
 *       不引入硬编码主频；P2 由 TIMG12 1ms 节拍替换本函数。
 */
static void delay_ms_busywait(uint32_t ms)
{
    const uint32_t cycles_per_ms = CPUCLK_FREQ / 1000u;   /* 由 CPUCLK_FREQ 生成宏自动计算（当前 80MHz 基线 = 80000），不引入硬编码主频 */
    while (ms-- > 0u) {
        delay_cycles(cycles_per_ms);
    }
}

/** 输出一次完整启动信息（每次复位只输出一次；P1 验收项） */
static void print_boot_banner(void)
{
    uart0_print("\r\n=== MSPM0G3507 control-lib bringup ===\r\n");
    uart0_print("fw   : v" FW_VERSION_STR "\r\n");
    uart0_print("app  : LED_BRINGUP (id=");
    uart0_print_u32((uint32_t)APP_SELECTED_ID);
    uart0_print(")\r\n");
    uart0_print("clk  : CPUCLK=");
    uart0_print_u32(CPUCLK_FREQ);
    uart0_print(" Hz\r\n");
    uart0_print("rst  : ");
    uart0_print(reset_reason_str(DL_SYSCTL_getResetCause()));
    uart0_print("\r\n");
    uart0_print("=====================================\r\n");
}

int main(void)
{
    /* 1. SysConfig 生成初始化：电源/时钟/LED/UART0（生成文件，禁手改） */
    SYSCFG_DL_init();

    /* 2. 执行器安全：P1 无 PWM 定时器/电机/舵机代码，执行器物理隔离，天然禁用 */

    /* 3+4. 启动日志（UART0 @ PA10/PA11 → 板载 CH340 → PC 串口） */
    print_boot_banner();

    /* 5. LED 心跳：PB22 1Hz 翻转（500ms 亮 / 500ms 灭） */
    for (;;) {
        DL_GPIO_togglePins(LED_PORT, LED_PIN_PIN);
        delay_ms_busywait(500u);
    }
}
