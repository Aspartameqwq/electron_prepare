/**
 * @file project_config.h
 * @brief 软件策略参数（唯一编译期来源）
 *
 * 职责（见 PLAN.md §1）：
 *   - 只存放"软件 / 协议 / 策略"参数（超时、容量、周期、阈值等）。
 *   - 物理硬件参数在 hardware_config.h；当前构建选择在 app_config.h。
 *   - UNKNOWN 参数不得以猜测值写入（用各模块 *_CONFIG_READY 标志 + #error）。
 *
 * 当前阶段（P3-SOFTWARE）：仅帧协议默认超时。
 */
#ifndef PROJECT_CONFIG_H_
#define PROJECT_CONFIG_H_

#include <stdint.h>

/* ---- 帧协议超时默认值（frame_codec，可按链路覆盖） ---- */
#define FRAME_INTER_BYTE_TIMEOUT_MS_DEFAULT  100u   /* 字节间隔超时默认（ms） */
#define FRAME_TOTAL_TIMEOUT_MS_DEFAULT       500u   /* 总组帧超时默认（ms），须 > 字节间隔 */
/* 注：实际链路（如 9600 波特 HC-04 桥接）应按下式计算覆盖：
 *     frame_time_ms = ceil(帧字节数 * 10 * 1000 / 波特率)
 *     本默认值仅适用于 115200 级别的高速链路。 */

#include "config_validate.h"   /* 编译期合法性检查 */

#endif /* PROJECT_CONFIG_H_ */
