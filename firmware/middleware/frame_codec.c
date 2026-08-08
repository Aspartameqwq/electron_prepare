/**
 * @file frame_codec.c
 * @brief 帧协议 v1 编解码实现（纯算法，无硬件依赖）
 *
 * 解析器为逐字节状态机，错误恢复采用"SOF 重叠保留"规则：
 *   - 出错/超时复位后，若当前字节 == SOF1(0xA5)，保留该字节为新帧首字节，
 *     从而正确处理线上连续的 `A5 A5 5A ...`。
 *   - CRC 在收集 VERSION..PAYLOAD 时增量累加，收齐 CRC 字段后一次性比对。
 */
#include "frame_codec.h"
#include <string.h>

/* 内部解析状态机 */
typedef enum {
    ST_IDLE,      /* 等待 SOF1 */
    ST_SOF2,      /* 已收 SOF1，等待 SOF2 */
    ST_VERSION,   /* 版本 */
    ST_TYPE,      /* 类型 */
    ST_FLAGS,     /* 标志 */
    ST_SEQ,       /* 序号 */
    ST_LEN_LO,    /* 长度低字节 */
    ST_LEN_HI,    /* 长度高字节 */
    ST_PAYLOAD,   /* 载荷 */
    ST_CRC_LO,    /* CRC 低字节 */
    ST_CRC_HI     /* CRC 高字节（收齐后校验交付） */
} parser_state_t;

/** CRC-16/CCITT-FALSE 单字节推进（MSB-first）。 */
static uint16_t crc16_update(uint16_t crc, uint8_t byte)
{
    crc ^= (uint16_t)byte << 8u;
    for (int i = 0; i < 8; i++) {
        crc = (crc & 0x8000u) ? (uint16_t)((crc << 1u) ^ 0x1021u)
                              : (uint16_t)(crc << 1u);
    }
    return crc;
}

uint16_t frame_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = CRC16_CCITT_INIT;
    for (size_t i = 0; i < len; i++) {
        crc = crc16_update(crc, data[i]);
    }
    return crc;
}

bool frame_parser_init(frame_parser_t *p, uint32_t inter_byte_ms, uint32_t total_ms)
{
    /* 参数校验：p 非空；超时须满足 0 < inter_byte < total（否则拒绝，不进入可用状态） */
    if (p == NULL || inter_byte_ms == 0u || total_ms <= inter_byte_ms) {
        return false;
    }
    frame_parser_reset(p);
    p->inter_byte_timeout_ms = inter_byte_ms;
    p->total_timeout_ms      = total_ms;
    return true;
}

void frame_parser_reset(frame_parser_t *p)
{
    if (p == NULL) {           /* 防御：NULL 安全无操作 */
        return;
    }
    p->state        = ST_IDLE;
    p->last_byte_ms = 0u;
    p->frame_start_ms = 0u;
    p->type         = 0u;
    p->flags        = 0u;
    p->seq          = 0u;
    p->length       = 0u;
    p->payload_len  = 0u;
    p->crc_accum    = CRC16_CCITT_INIT;
    p->rx_crc       = 0u;
}

/**
 * @brief 出错后复位，并按 SOF 重叠规则决定是否保留当前字节为新帧首字节。
 * @return 传入的 err（调用方据此上报错误；解析器已复位并可继续收下一帧）。
 */
static frame_status_t frame_resync(frame_parser_t *p, uint8_t byte,
                                   uint32_t now_ms, frame_status_t err)
{
    frame_parser_reset(p);
    p->last_byte_ms   = now_ms;
    p->frame_start_ms = now_ms;
    if (byte == FRAME_SOF1) {
        /* 重叠：当前字节即新帧 SOF1，直接进入等 SOF2 状态 */
        p->state = ST_SOF2;
    }
    return err;
}

frame_status_t frame_parser_feed(frame_parser_t *p, uint8_t byte,
                                 uint32_t now_ms, frame_t *out)
{
    if (p == NULL) {                       /* 防御：非法参数 */
        return FRAME_ERR_INVALID_ARG;
    }
    /* 帧中途先查超时（字节间隔 / 总组帧），任一命中即复位并重叠重同步 */
    if (p->state != ST_IDLE) {
        if ((int32_t)(now_ms - p->last_byte_ms) > (int32_t)p->inter_byte_timeout_ms) {
            return frame_resync(p, byte, now_ms, FRAME_ERR_TIMEOUT);
        }
        if ((int32_t)(now_ms - p->frame_start_ms) > (int32_t)p->total_timeout_ms) {
            return frame_resync(p, byte, now_ms, FRAME_ERR_TOTAL_TIMEOUT);
        }
    }
    p->last_byte_ms = now_ms;

    switch ((parser_state_t)p->state) {
    case ST_IDLE:
        if (byte == FRAME_SOF1) {
            p->state        = ST_SOF2;
            p->frame_start_ms = now_ms;     /* 帧计时起点 */
        }
        return FRAME_INCOMPLETE;

    case ST_SOF2:
        if (byte == FRAME_SOF2) {
            p->state = ST_VERSION;
        } else if (byte == FRAME_SOF1) {
            p->frame_start_ms = now_ms;     /* A5 A5 5A：连续 SOF1，保持等待 SOF2 */
        } else {
            return frame_resync(p, byte, now_ms, FRAME_ERR_INVALID_ARG);
        }
        return FRAME_INCOMPLETE;

    case ST_VERSION:
        if (byte != FRAME_VERSION) {
            return frame_resync(p, byte, now_ms, FRAME_ERR_VERSION);
        }
        p->crc_accum = crc16_update(p->crc_accum, byte);   /* CRC 范围从 VERSION 起 */
        p->state = ST_TYPE;
        return FRAME_INCOMPLETE;

    case ST_TYPE:
        p->type = byte;
        p->crc_accum = crc16_update(p->crc_accum, byte);
        p->state = ST_FLAGS;
        return FRAME_INCOMPLETE;

    case ST_FLAGS:
        if (byte & FRAME_FLAG_RESERVED_MASK) {
            return frame_resync(p, byte, now_ms, FRAME_ERR_RESERVED_FLAGS);
        }
        p->flags = byte;
        p->crc_accum = crc16_update(p->crc_accum, byte);
        p->state = ST_SEQ;
        return FRAME_INCOMPLETE;

    case ST_SEQ:
        p->seq = byte;
        p->crc_accum = crc16_update(p->crc_accum, byte);
        p->state = ST_LEN_LO;
        return FRAME_INCOMPLETE;

    case ST_LEN_LO:
        p->length = byte;                                  /* 长度低字节 */
        p->crc_accum = crc16_update(p->crc_accum, byte);
        p->state = ST_LEN_HI;
        return FRAME_INCOMPLETE;

    case ST_LEN_HI:
        p->length |= (uint16_t)byte << 8u;                 /* 长度高字节（小端） */
        p->crc_accum = crc16_update(p->crc_accum, byte);
        if (p->length > FRAME_PAYLOAD_MAX) {
            return frame_resync(p, byte, now_ms, FRAME_ERR_LENGTH);
        }
        p->payload_len = 0u;
        p->state = (p->length == 0u) ? ST_CRC_LO : ST_PAYLOAD;
        return FRAME_INCOMPLETE;

    case ST_PAYLOAD:
        p->payload[p->payload_len++] = byte;
        p->crc_accum = crc16_update(p->crc_accum, byte);
        if (p->payload_len >= p->length) {
            p->state = ST_CRC_LO;
        }
        return FRAME_INCOMPLETE;

    case ST_CRC_LO:
        p->rx_crc = byte;                                  /* CRC 低字节 */
        p->state  = ST_CRC_HI;
        return FRAME_INCOMPLETE;

    case ST_CRC_HI: {
        p->rx_crc |= (uint16_t)byte << 8u;                 /* CRC 高字节 */
        const uint16_t crc_ok = p->crc_accum;
        const uint16_t rx_crc = p->rx_crc;
        const uint16_t f_len  = p->length;
        /* 校验通过才交付，避免 CRC 错误时向 out 写入"假帧"；
         * frame_parser_reset 只清 payload_len、不清 payload 数组内容，故可先拷贝。 */
        if (rx_crc == crc_ok && out != NULL) {
            out->type   = p->type;
            out->flags  = p->flags;
            out->seq    = p->seq;
            out->length = f_len;
            memcpy(out->payload, p->payload, f_len);
        }
        frame_parser_reset(p);                               /* 无论对错都复位，继续收下一帧 */
        return (rx_crc == crc_ok) ? FRAME_COMPLETE : FRAME_ERR_CRC;
    }

    default:
        return frame_resync(p, byte, now_ms, FRAME_ERR_INVALID_ARG);
    }
}

frame_status_t frame_parser_poll_timeout(frame_parser_t *p, uint32_t now_ms)
{
    if (p == NULL) {                       /* 防御：非法参数 */
        return FRAME_ERR_INVALID_ARG;
    }
    if (p->state == ST_IDLE) {
        return FRAME_INCOMPLETE;
    }
    if ((int32_t)(now_ms - p->last_byte_ms) > (int32_t)p->inter_byte_timeout_ms) {
        frame_parser_reset(p);
        return FRAME_ERR_TIMEOUT;
    }
    if ((int32_t)(now_ms - p->frame_start_ms) > (int32_t)p->total_timeout_ms) {
        frame_parser_reset(p);
        return FRAME_ERR_TOTAL_TIMEOUT;
    }
    return FRAME_INCOMPLETE;
}

frame_status_t frame_encode(const frame_t *frame, uint8_t *out,
                            size_t out_cap, size_t *out_len)
{
    if (frame == NULL || out == NULL || out_len == NULL) {
        return FRAME_ERR_INVALID_ARG;
    }
    *out_len = 0u;                       /* 失败时保证清零（成功才写入实际长度） */
    if (frame->length > FRAME_PAYLOAD_MAX) {
        return FRAME_ERR_LENGTH;
    }
    if (frame->flags & FRAME_FLAG_RESERVED_MASK) {
        return FRAME_ERR_RESERVED_FLAGS;   /* 发送侧同样禁止保留位 */
    }
    const size_t total = FRAME_FIXED_HEADER + (size_t)frame->length + FRAME_CRC_BYTES;
    if (out_cap < total) {
        return FRAME_ERR_OUTPUT_CAPACITY;
    }

    uint8_t *o = out;
    *o++ = FRAME_SOF1;
    *o++ = FRAME_SOF2;

    uint16_t crc = CRC16_CCITT_INIT;
    const uint8_t ver = FRAME_VERSION;
    *o++ = ver;                                      crc = crc16_update(crc, ver);
    *o++ = frame->type;                              crc = crc16_update(crc, frame->type);
    *o++ = frame->flags;                             crc = crc16_update(crc, frame->flags);
    *o++ = frame->seq;                               crc = crc16_update(crc, frame->seq);
    *o++ = (uint8_t)(frame->length & 0xFFu);         crc = crc16_update(crc, (uint8_t)(frame->length & 0xFFu));
    *o++ = (uint8_t)((frame->length >> 8u) & 0xFFu); crc = crc16_update(crc, (uint8_t)((frame->length >> 8u) & 0xFFu));
    for (uint16_t i = 0; i < frame->length; i++) {
        *o++ = frame->payload[i];
        crc  = crc16_update(crc, frame->payload[i]);
    }
    *o++ = (uint8_t)(crc & 0xFFu);                   /* CRC 小端 */
    *o++ = (uint8_t)((crc >> 8u) & 0xFFu);

    *out_len = total;
    return FRAME_COMPLETE;
}
