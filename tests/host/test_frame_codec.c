/**
 * @file test_frame_codec.c
 * @brief frame_codec 纯算法 host 测试（PC 上运行，clang C11 -Werror）
 *
 * 覆盖：CRC 向量、空/小/最大载荷往返、编码错误、解析错误、
 *       SOF 重叠恢复、字节间隔/总组帧超时、固定种子随机往返、随机字节模糊。
 */
#include <stdio.h>
#include <string.h>
#include "frame_codec.h"
#include "project_config.h"   /* 触发 config_validate.h 编译期断言 */

static int g_fails = 0;
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);           \
            g_fails++;                                                       \
        }                                                                    \
    } while (0)

static uint32_t rng_state;
static uint32_t rng_next(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;   /* 简单 LCG，固定种子可复现 */
    return rng_state;
}

/** 逐字节喂入一段数据，返回首个 FRAME_COMPLETE 或首个错误。 */
static frame_status_t feed_all(frame_parser_t *p, const uint8_t *data,
                               size_t len, uint32_t *now, frame_t *out)
{
    frame_status_t st = FRAME_INCOMPLETE;
    for (size_t i = 0; i < len; i++) {
        st = frame_parser_feed(p, data[i], (*now)++, out);
        if (st == FRAME_COMPLETE || st != FRAME_INCOMPLETE) {
            return st;                              /* 完整帧或已出错，停止 */
        }
    }
    return st;
}

/** 构造一个帧并做 编码->解析 往返一致性检查。 */
static void test_roundtrip(const frame_t *f)
{
    uint8_t buf[FRAME_MAX_TOTAL];
    size_t len = 0;
    CHECK(frame_encode(f, buf, sizeof buf, &len) == FRAME_COMPLETE);
    CHECK(len == FRAME_FIXED_HEADER + (size_t)f->length + FRAME_CRC_BYTES);

    frame_parser_t p;
    frame_parser_init(&p, 100u, 500u);
    frame_t out;
    uint32_t now = 1000u;
    CHECK(feed_all(&p, buf, len, &now, &out) == FRAME_COMPLETE);
    CHECK(out.type == f->type);
    CHECK(out.flags == f->flags);
    CHECK(out.seq == f->seq);
    CHECK(out.length == f->length);
    CHECK(memcmp(out.payload, f->payload, f->length) == 0);
}

static void test_crc_vector(void)
{
    /* 已验证向量（见 PLAN.md §7）：VERSION=01 TYPE=10 FLAGS=00 SEQ=01 LENGTH=0000
     * CRC-16/CCITT-FALSE 结果应为 0x78DA。 */
    const uint8_t data[] = {0x01, 0x10, 0x00, 0x01, 0x00, 0x00};
    CHECK(frame_crc16(data, sizeof data) == 0x78DAu);
}

static void test_encode_errors(void)
{
    frame_t f;
    memset(&f, 0, sizeof f);
    uint8_t buf[FRAME_MAX_TOTAL];
    size_t len = 0;

    f.length = FRAME_PAYLOAD_MAX + 1u;                 /* 超长载荷 */
    CHECK(frame_encode(&f, buf, sizeof buf, &len) == FRAME_ERR_LENGTH);

    memset(&f, 0, sizeof f);
    f.flags = 0x08u;                                   /* 保留位非 0 */
    CHECK(frame_encode(&f, buf, sizeof buf, &len) == FRAME_ERR_RESERVED_FLAGS);

    memset(&f, 0, sizeof f);
    CHECK(frame_encode(&f, buf, 3u, &len) == FRAME_ERR_OUTPUT_CAPACITY);  /* 容量不足 */

    CHECK(frame_encode(NULL, buf, sizeof buf, &len) == FRAME_ERR_INVALID_ARG);
    CHECK(frame_encode(&f, NULL, sizeof buf, &len) == FRAME_ERR_INVALID_ARG);
}

static void test_parse_errors(void)
{
    frame_parser_t p;
    frame_parser_init(&p, 100u, 500u);
    frame_t out;
    uint32_t now = 1000u;

    const uint8_t bad_version[] = {FRAME_SOF1, FRAME_SOF2, 0x02, 0, 0, 0, 0, 0};
    CHECK(feed_all(&p, bad_version, sizeof bad_version, &now, &out) == FRAME_ERR_VERSION);

    const uint8_t bad_flags[] = {FRAME_SOF1, FRAME_SOF2, FRAME_VERSION, 0, 0x08, 0, 0, 0};
    CHECK(feed_all(&p, bad_flags, sizeof bad_flags, &now, &out) == FRAME_ERR_RESERVED_FLAGS);

    const uint8_t bad_len[] = {FRAME_SOF1, FRAME_SOF2, FRAME_VERSION, 0, 0, 0, 0x81, 0};
    CHECK(feed_all(&p, bad_len, sizeof bad_len, &now, &out) == FRAME_ERR_LENGTH);

    /* 合法帧篡改 CRC 低字节 */
    frame_t f;
    memset(&f, 0, sizeof f);
    f.length = 3u;
    f.payload[0] = 1u; f.payload[1] = 2u; f.payload[2] = 3u;
    uint8_t buf[FRAME_MAX_TOTAL];
    size_t len = 0;
    CHECK(frame_encode(&f, buf, sizeof buf, &len) == FRAME_COMPLETE);
    buf[len - 2u] ^= 0xFFu;
    CHECK(feed_all(&p, buf, len, &now, &out) == FRAME_ERR_CRC);
}

static void test_sof_overlap(void)
{
    /* 帧前插入一个多余 0xA5，应正确识别 A5 A5 5A 并解析出完整帧 */
    frame_t f;
    memset(&f, 0, sizeof f);
    f.type = 0x10u;
    uint8_t buf[FRAME_MAX_TOTAL];
    size_t len = 0;
    CHECK(frame_encode(&f, buf, sizeof buf, &len) == FRAME_COMPLETE);

    uint8_t stream[FRAME_MAX_TOTAL + 1u];
    stream[0] = FRAME_SOF1;
    memcpy(stream + 1, buf, len);

    frame_parser_t p;
    frame_parser_init(&p, 100u, 500u);
    frame_t out;
    uint32_t now = 1000u;
    CHECK(feed_all(&p, stream, sizeof stream, &now, &out) == FRAME_COMPLETE);
    CHECK(out.type == 0x10u);
}

static void test_timeouts(void)
{
    frame_t out;
    uint32_t now = 1000u;

    /* 总组帧超时：inter_byte 设大、total 设小，隔离验证总超时路径 */
    frame_parser_t p;
    frame_parser_init(&p, 10000u, 100u);
    CHECK(frame_parser_feed(&p, FRAME_SOF1, now, &out) == FRAME_INCOMPLETE);
    CHECK(frame_parser_feed(&p, FRAME_SOF2, now + 1u, &out) == FRAME_INCOMPLETE);
    /* now=1000, frame_start=1000, last=1001；在 now+150=1150 时 (1150-1000)=150>100 → 总超时 */
    CHECK(frame_parser_poll_timeout(&p, now + 150u) == FRAME_ERR_TOTAL_TIMEOUT);

    /* 字节间隔超时：inter_byte 设小、total 设大，隔离验证间隔超时路径 */
    frame_parser_init(&p, 100u, 10000u);
    CHECK(frame_parser_feed(&p, FRAME_SOF1, now, &out) == FRAME_INCOMPLETE);
    CHECK(frame_parser_feed(&p, FRAME_SOF2, now + 1u, &out) == FRAME_INCOMPLETE);
    CHECK(frame_parser_feed(&p, FRAME_VERSION, now + 2u, &out) == FRAME_INCOMPLETE);
    /* last=1002；在 now+2+101=1103 时 (1103-1002)=101>100 → 字节间隔超时 */
    CHECK(frame_parser_poll_timeout(&p, now + 2u + 101u) == FRAME_ERR_TIMEOUT);
}

static void test_random_roundtrip(void)
{
    rng_state = 0x12345678u;
    printf("frame_codec random seed: 0x%08X\n", rng_state);
    frame_parser_t p;
    frame_parser_init(&p, 100u, 500u);

    for (int i = 0; i < 10000; i++) {
        frame_t f;
        memset(&f, 0, sizeof f);
        f.type   = (uint8_t)(rng_next() & 0xFFu);
        f.flags  = (uint8_t)(rng_next() & 0x07u);          /* 只生成合法位 */
        f.seq    = (uint8_t)(rng_next() & 0xFFu);
        f.length = (uint16_t)(rng_next() % (FRAME_PAYLOAD_MAX + 1u));
        for (uint16_t j = 0; j < f.length; j++) {
            f.payload[j] = (uint8_t)(rng_next() & 0xFFu);
        }

        uint8_t buf[FRAME_MAX_TOTAL];
        size_t len = 0;
        CHECK(frame_encode(&f, buf, sizeof buf, &len) == FRAME_COMPLETE);

        frame_t out;
        uint32_t now = 1000u + (uint32_t)i * 2u;
        CHECK(feed_all(&p, buf, len, &now, &out) == FRAME_COMPLETE);
        CHECK(out.type == f.type && out.flags == f.flags && out.seq == f.seq);
        CHECK(out.length == f.length);
        CHECK(memcmp(out.payload, f.payload, f.length) == 0);
    }
}

static void test_fuzz(void)
{
    rng_state = 0x9E3779B9u;
    printf("frame_codec fuzz seed: 0x%08X\n", rng_state);
    frame_parser_t p;
    frame_parser_init(&p, 100u, 500u);
    frame_t out;
    uint32_t now = 1000u;

    for (int i = 0; i < 100000; i++) {
        const uint8_t b = (uint8_t)(rng_next() & 0xFFu);
        (void)frame_parser_feed(&p, b, now++, &out);       /* 任意返回码都应安全处理 */
        if ((i % 500) == 0) {
            (void)frame_parser_poll_timeout(&p, now);      /* 周期性检查超时 */
        }
    }

    /* 结束后解析器必须仍可用：能正常解析一帧 */
    frame_t f;
    memset(&f, 0, sizeof f);
    uint8_t buf[FRAME_MAX_TOTAL];
    size_t len = 0;
    CHECK(frame_encode(&f, buf, sizeof buf, &len) == FRAME_COMPLETE);
    CHECK(feed_all(&p, buf, len, &now, &out) == FRAME_COMPLETE);
}

int main(void)
{
    test_crc_vector();

    /* 空载荷 */
    frame_t f0;
    memset(&f0, 0, sizeof f0);
    test_roundtrip(&f0);

    /* 小载荷 + 各标志位 */
    frame_t f1;
    memset(&f1, 0, sizeof f1);
    f1.type   = 0x10u;
    f1.flags  = FRAME_FLAG_ACK_REQUIRED;
    f1.seq    = 42u;
    f1.length = 5u;
    memcpy(f1.payload, "hello", 5);
    test_roundtrip(&f1);

    /* 最大载荷 */
    frame_t f2;
    memset(&f2, 0, sizeof f2);
    f2.length = FRAME_PAYLOAD_MAX;
    for (uint16_t j = 0; j < f2.length; j++) {
        f2.payload[j] = (uint8_t)(j * 7u);
    }
    test_roundtrip(&f2);

    test_encode_errors();
    test_parse_errors();
    test_sof_overlap();
    test_timeouts();
    test_random_roundtrip();
    test_fuzz();

    if (g_fails) {
        printf("frame_codec test: %d FAILURE(S)\n", g_fails);
        return 1;
    }
    printf("frame_codec test: ALL PASSED\n");
    return 0;
}
