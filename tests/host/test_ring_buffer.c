/**
 * @file test_ring_buffer.c
 * @brief ring_buffer 纯算法 host 测试（PC 上运行，clang C11 -Werror）
 *
 * 覆盖：初始化空态、FIFO 顺序、满态（保留一空槽）、索引回绕、
 *       固定种子随机 push/pop 与参考模型比对。
 */
#include <stdio.h>
#include <stdint.h>
#include "ring_buffer.h"

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

static void test_basic(void)
{
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    CHECK(ring_buffer_is_empty(&rb));
    CHECK(ring_buffer_available(&rb) == 0u);
    CHECK(ring_buffer_free(&rb) == RING_BUFFER_CAPACITY - 1u);

    /* FIFO 顺序 */
    uint8_t b;
    CHECK(ring_buffer_push(&rb, 0x11u));
    CHECK(ring_buffer_push(&rb, 0x22u));
    CHECK(ring_buffer_push(&rb, 0x33u));
    CHECK(ring_buffer_pop(&rb, &b) && b == 0x11u);
    CHECK(ring_buffer_pop(&rb, &b) && b == 0x22u);
    CHECK(ring_buffer_pop(&rb, &b) && b == 0x33u);
    CHECK(ring_buffer_is_empty(&rb));
    CHECK(!ring_buffer_pop(&rb, &b));                  /* 空时出队失败 */
}

static void test_full(void)
{
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    /* 实际可用 = 容量 - 1（保留一空槽） */
    for (size_t i = 0; i < RING_BUFFER_CAPACITY - 1u; i++) {
        CHECK(ring_buffer_push(&rb, (uint8_t)i));
    }
    CHECK(ring_buffer_is_full(&rb));
    CHECK(!ring_buffer_push(&rb, 0xFFu));              /* 满则拒绝 */

    uint8_t b;
    CHECK(ring_buffer_pop(&rb, &b));                   /* 弹出一个后可再入队 */
    CHECK(ring_buffer_push(&rb, 0xEEu));
    CHECK(ring_buffer_available(&rb) == RING_BUFFER_CAPACITY - 1u);
}

static void test_wrap(void)
{
    ring_buffer_t rb;
    ring_buffer_init(&rb);

    /* 填满再清空，制造 head/tail 索引回绕 */
    for (size_t i = 0; i < RING_BUFFER_CAPACITY - 1u; i++) {
        CHECK(ring_buffer_push(&rb, (uint8_t)i));
    }
    for (size_t i = 0; i < RING_BUFFER_CAPACITY - 1u; i++) {
        uint8_t b;
        CHECK(ring_buffer_pop(&rb, &b));
        CHECK(b == (uint8_t)i);
    }
    CHECK(ring_buffer_is_empty(&rb));

    /* 再次填充（此时 head 已回绕），顺序仍正确 */
    for (size_t i = 0; i < 10u; i++) {
        CHECK(ring_buffer_push(&rb, (uint8_t)(0xA0u + i)));
    }
    for (size_t i = 0; i < 10u; i++) {
        uint8_t b;
        CHECK(ring_buffer_pop(&rb, &b));
        CHECK(b == (uint8_t)(0xA0u + i));
    }
}

static void test_model(void)
{
    /* 随机 push/pop，与独立参考队列逐字节比对 */
    rng_state = 0xDEADBEEFu;
    printf("ring_buffer model seed: 0x%08X\n", rng_state);

    ring_buffer_t rb;
    ring_buffer_init(&rb);

    uint8_t model[RING_BUFFER_CAPACITY];
    size_t m_head = 0, m_tail = 0, m_count = 0;

    for (int i = 0; i < 100000; i++) {
        const uint32_t r = rng_next();
        if ((r & 3u) < 2u && m_count < RING_BUFFER_CAPACITY - 1u) {
            const uint8_t v = (uint8_t)(rng_next() & 0xFFu);
            CHECK(ring_buffer_push(&rb, v));
            model[m_head] = v;
            m_head = (m_head + 1u) % RING_BUFFER_CAPACITY;
            m_count++;
        } else if (m_count > 0) {
            uint8_t v;
            CHECK(ring_buffer_pop(&rb, &v));
            CHECK(v == model[m_tail]);
            m_tail = (m_tail + 1u) % RING_BUFFER_CAPACITY;
            m_count--;
        }
        if ((i % 1000) == 0) {
            CHECK(ring_buffer_available(&rb) == m_count);
            CHECK(ring_buffer_is_empty(&rb) == (m_count == 0u));
        }
    }
}

static void test_null_safety(void)
{
    uint8_t b = 0u;
    ring_buffer_t rb;
    CHECK(!ring_buffer_push(NULL, 0x11u));      /* NULL rb → false */
    CHECK(!ring_buffer_pop(NULL, &b));           /* NULL rb → false */
    CHECK(!ring_buffer_pop(&rb, NULL));          /* NULL byte → false */
    CHECK(ring_buffer_available(NULL) == 0u);
    CHECK(ring_buffer_free(NULL) == 0u);
    CHECK(ring_buffer_is_empty(NULL));           /* NULL → true（视为空） */
    CHECK(!ring_buffer_is_full(NULL));
    ring_buffer_init(NULL);                      /* 不崩溃 */
    ring_buffer_reset(NULL);                     /* 不崩溃 */
}

int main(void)
{
    test_basic();
    test_full();
    test_wrap();
    test_model();
    test_null_safety();

    if (g_fails) {
        printf("ring_buffer test: %d FAILURE(S)\n", g_fails);
        return 1;
    }
    printf("ring_buffer test: ALL PASSED\n");
    return 0;
}
