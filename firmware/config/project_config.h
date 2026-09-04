/**
 * @file project_config.h
 * @brief 软件策略参数（唯一编译期来源）
 *
 * 职责（见 PLAN.md §1）：
 *   - 只存放"软件 / 协议 / 策略"参数（超时、容量、周期、阈值等）。
 *   - 物理硬件参数在 hardware_config.h；当前构建选择在 app_config.h。
 *   - UNKNOWN 参数不得以猜测值写入（用各模块 *_CONFIG_READY 标志 + #error）。
 *
 * 当前包含 P2 调度验收策略及帧协议预研默认超时；均不声明未知物理参数。
 */
#ifndef PROJECT_CONFIG_H_
#define PROJECT_CONFIG_H_

#include <stdint.h>

/* P2 专用策略（DESIGN_DEFAULT，单位见宏名）；1ms 合成任务不用于 P10。 */
#define P2_TEST_DURATION_MS             600000u /* PLAN P2：10min */
#define P2_TASK_FAST_PERIOD_MS          1u
#define P2_TASK_MEDIUM_PERIOD_MS        10u
#define P2_TASK_SLOW_PERIOD_MS          20u
#define P2_REPORT_PERIOD_MS             10000u /* 每 10s 一条进度，最终结果每 10s 重发 */
#define P2_LED_HALF_PERIOD_MS           500u   /* 1Hz 心跳 */
#define P2_CONSOLE_CAPACITY             512u   /* 单消息字节容量，完整复制或拒绝 */
#define P2_CONSOLE_BYTES_PER_SERVICE    16u    /* 单次服务最大写 FIFO 字节数 */
#define TIMEBASE_SELFCHECK_WAIT_MS      4u     /* 启动自检忙等上限的名义毫秒数 */
#define TIMEBASE_SELFCHECK_MIN_TICKS    2u     /* 允许启动相位误差 */

/* ---- 帧协议超时默认值（frame_codec，可按链路覆盖） ---- */
#define FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT  100u   /* 字节间隔超时默认（ms） */
#define FRAME_TOTAL_TIMEOUT_MS_DEFAULT       500u   /* 总组帧超时默认（ms），须 > 字节间隔 */
/* 注：实际链路（如 9600 波特 HC-04 桥接）应按下式计算覆盖：
 *     frame_time_ms = ceil(帧字节数 * 10 * 1000 / 波特率)
 *     本默认值仅适用于 115200 级别的高速链路。 */

#include "config_validate.h"   /* 编译期合法性检查 */

#endif /* PROJECT_CONFIG_H_ */
