/**
 * @file ring_buffer.c
 * @brief SPSC 环形缓冲区实现
 *
 * 实现要点（与 PLAN.md §7 对齐）：
 *   - head/tail 均为容量内的掩码索引；head==tail 空，next_head==tail 满；
 *   - 发布顺序：生产者先写 buffer 再写 head；消费者先读 head 再读 buffer 再写 tail；
 *   - 不自行开关全局中断；并发安全依赖 SPSC 假设 + 对齐 volatile 访问原子性。
 */
#include "ring_buffer.h"

void ring_buffer_init(ring_buffer_t *rb)
{
    rb->head = 0u;
    rb->tail = 0u;
}

bool ring_buffer_push(ring_buffer_t *rb, uint8_t byte)
{
    const uint32_t tail = rb->tail;                     /* 读一次消费者游标快照（消费者是唯一写者） */
    const uint32_t next = (rb->head + 1u) & (RING_BUFFER_CAPACITY - 1u);
    if (next == tail) {
        return false;                                   /* 满：保留一个空槽，拒绝写入 */
    }
    rb->buffer[rb->head] = byte;                        /* 先写数据…… */
    rb->head = next;                                    /* ……再发布 head（发布顺序） */
    return true;
}

bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *byte)
{
    if (rb->head == rb->tail) {
        return false;                                   /* 空 */
    }
    *byte = rb->buffer[rb->tail];                       /* 先读数据…… */
    rb->tail = (rb->tail + 1u) & (RING_BUFFER_CAPACITY - 1u);  /* ……再发布 tail */
    return true;
}

size_t ring_buffer_available(const ring_buffer_t *rb)
{
    return (size_t)((rb->head - rb->tail) & (RING_BUFFER_CAPACITY - 1u));
}

size_t ring_buffer_free(const ring_buffer_t *rb)
{
    return (RING_BUFFER_CAPACITY - 1u) - ring_buffer_available(rb);
}

bool ring_buffer_is_empty(const ring_buffer_t *rb)
{
    return (rb->head == rb->tail);
}

bool ring_buffer_is_full(const ring_buffer_t *rb)
{
    return ring_buffer_free(rb) == 0u;
}

void ring_buffer_reset(ring_buffer_t *rb)
{
    rb->head = 0u;
    rb->tail = 0u;
}
