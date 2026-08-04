/**
 * @file frame_codec.h
 * @brief UART1 帧协议 v1 的纯算法编解码模块
 *
 * 功能：
 *   - frame_encode()：逻辑帧 → 线上字节流（显式 little-endian，禁止 packed 结构体序列化）
 *   - frame_parser_*()：逐字节解析，含 CRC 校验、长度/版本/保留位检查、双超时、SOF 重叠恢复
 *
 * 依赖：纯 C11（stdint/stddef/stdbool/string），不含 MSPM0 SDK 头，可在 PC 上 host 编译测试。
 * 职责边界：只做字节编解码与解析；不负责 ACK/NACK/重发/去重（属 E1 的 protocol_session）。
 *
 * 典型用法：
 *   frame_parser_t p;
 *   frame_parser_init(&p, inter_byte_ms, total_ms);
 *   // 每收到一个字节：
 *   if (frame_parser_feed(&p, byte, now_ms, &frame) == FRAME_COMPLETE) { ... 处理 frame ... }
 *   // 无字节到达时周期性调用：
 *   (void)frame_parser_poll_timeout(&p, now_ms);
 *
 * 帧格式 v1（见 PLAN.md §7）：
 *   SOF1(0xA5) SOF2(0x5A) VERSION(0x01) TYPE FLAGS SEQ LENGTH(2LE) PAYLOAD(0~128) CRC16(2LE)
 *   - FLAGS：bit0=ACK_REQUIRED bit1=RESPONSE bit2=ERROR，bit3-7 保留须为 0
 *   - CRC16：CRC-16/CCITT-FALSE（poly 0x1021, init 0xFFFF, 不反射, xorout 0x0000），
 *     范围 = VERSION 至 PAYLOAD 的全部字节；线上小端发送。
 *   - 最大帧 = 2+1+1+1+1+2+128+2 = 138 字节。
 */
#ifndef FRAME_CODEC_H_
#define FRAME_CODEC_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ---- 协议常量 ---- */
#define FRAME_SOF1              0xA5u   /* 帧头 1 */
#define FRAME_SOF2              0x5Au   /* 帧头 2 */
#define FRAME_VERSION           0x01u   /* 协议版本（v1） */
#define FRAME_PAYLOAD_MAX       128u    /* 载荷最大字节数 */
#define FRAME_FIXED_HEADER      8u      /* SOF1+SOF2+VERSION+TYPE+FLAGS+SEQ+LENGTH(2) */
#define FRAME_CRC_BYTES         2u
#define FRAME_MAX_TOTAL         (FRAME_FIXED_HEADER + FRAME_PAYLOAD_MAX + FRAME_CRC_BYTES) /* 138 */
#define CRC16_CCITT_INIT        0xFFFFu

#define FRAME_FLAG_ACK_REQUIRED (1u << 0)   /* bit0：要求 ACK */
#define FRAME_FLAG_RESPONSE     (1u << 1)   /* bit1：响应帧 */
#define FRAME_FLAG_ERROR        (1u << 2)   /* bit2：错误帧 */
#define FRAME_FLAG_RESERVED_MASK 0xF8u      /* bit3-7 保留，收发均须为 0 */

/** 逻辑帧（编解码的中间表示，不直接对应线上字节布局） */
typedef struct {
    uint8_t  type;                          /* 消息类型 */
    uint8_t  flags;                         /* FLAGS 位定义见上 */
    uint8_t  seq;                           /* 发送序号 0~255 */
    uint16_t length;                        /* 载荷长度 */
    uint8_t  payload[FRAME_PAYLOAD_MAX];    /* 载荷 */
} frame_t;

/** 帧处理结果状态码 */
typedef enum {
    FRAME_COMPLETE,             /* 完整帧：解析成功（或编码成功） */
    FRAME_INCOMPLETE,           /* 尚未凑够一帧，需更多字节 */
    FRAME_ERR_LENGTH,           /* 长度非法（>128） */
    FRAME_ERR_CRC,              /* CRC 校验失败 */
    FRAME_ERR_RESERVED_FLAGS,   /* FLAGS 保留位非 0 */
    FRAME_ERR_VERSION,          /* VERSION 非 0x01 */
    FRAME_ERR_TIMEOUT,          /* 字节间隔超时 */
    FRAME_ERR_TOTAL_TIMEOUT,    /* 总组帧超时 */
    FRAME_ERR_OUTPUT_CAPACITY,  /* 输出缓冲区容量不足 */
    FRAME_ERR_INVALID_ARG       /* 参数非法（空指针等） */
} frame_status_t;

/**
 * @brief 帧解析器实例（调用方静态分配）。
 * @note 内部字段由 frame_codec.c 维护，调用方只应通过接口操作；state 为内部状态机，勿直接读写。
 */
typedef struct {
    uint32_t inter_byte_timeout_ms;         /* 字节间隔超时（ms） */
    uint32_t total_timeout_ms;              /* 总组帧超时（ms） */
    uint32_t last_byte_ms;                  /* 最近一个字节的时间戳 */
    uint32_t frame_start_ms;                /* 收到 SOF1 的时间戳 */
    uint8_t  type;                          /* 正在解析的帧字段 */
    uint8_t  flags;
    uint8_t  seq;
    uint16_t length;
    uint8_t  payload[FRAME_PAYLOAD_MAX];
    uint16_t payload_len;                   /* 已收载荷字节数 */
    uint16_t crc_accum;                     /* 增量 CRC（覆盖 VERSION..PAYLOAD） */
    uint16_t rx_crc;                        /* 收到的 CRC（先低后高） */
    int      state;                         /* 内部状态机（见 frame_codec.c），勿直接操作 */
} frame_parser_t;

/** 初始化解析器。超时参数可按链路计算后传入。 */
void frame_parser_init(frame_parser_t *p, uint32_t inter_byte_ms, uint32_t total_ms);

/** 复位解析器（用于 RX 重同步临界区等）。 */
void frame_parser_reset(frame_parser_t *p);

/**
 * @brief 喂入一个字节。
 * @param out 解析出完整帧时写入（可为 NULL，此时仅校验不取帧）。
 * @return FRAME_COMPLETE（out 已被填充）/ FRAME_INCOMPLETE / 各类错误码。
 * @note 错误返回后解析器已复位；若触发错误的是 0xA5，会按 SOF 重叠规则保留为新帧首字节。
 */
frame_status_t frame_parser_feed(frame_parser_t *p, uint8_t byte, uint32_t now_ms, frame_t *out);

/**
 * @brief 无新字节到达时周期性调用，检查字节间隔 / 总组帧超时。
 * @return FRAME_INCOMPLETE（正常）/ FRAME_ERR_TIMEOUT / FRAME_ERR_TOTAL_TIMEOUT（已复位）。
 */
frame_status_t frame_parser_poll_timeout(frame_parser_t *p, uint32_t now_ms);

/**
 * @brief 把逻辑帧编码为线上字节流（显式 little-endian，非 packed 序列化）。
 * @param out 输出缓冲；@param out_cap 容量；@param out_len 成功时实际长度。
 * @return FRAME_COMPLETE（成功）或错误码。
 */
frame_status_t frame_encode(const frame_t *frame, uint8_t *out, size_t out_cap, size_t *out_len);

/** CRC-16/CCITT-FALSE（poly 0x1021, init 0xFFFF, 不反射, xorout 0）。供测试与 E1 去重键复用。 */
uint16_t frame_crc16(const uint8_t *data, size_t len);

#endif /* FRAME_CODEC_H_ */
