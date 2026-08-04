/**
 * @file ring_buffer.h
 * @brief 单生产者 / 单消费者（SPSC）无锁字节环形缓冲区
 *
 * 功能：固定容量字节环形缓冲，用于 UART RX/TX 等"ISR 生产、主循环消费"（或反之）场景。
 * 依赖：无（纯 C11，仅 stdint/stddef/stdbool），可在 PC 上 host 编译测试。
 *
 * 典型用法：
 *   ring_buffer_t rb;
 *   ring_buffer_init(&rb);
 *   ring_buffer_push(&rb, byte);   // 生产者（如 UART ISR）
 *   ring_buffer_pop(&rb, &byte);   // 消费者（如主循环）
 *
 * 并发约束（SPSC，见 PLAN.md §7）：
 *   - 一个缓冲区只允许一个生产者、一个消费者；
 *   - 生产者只写 head，消费者只写 tail（volatile uint32_t，对齐访问在 M0+/x86 上原子）；
 *   - 发布顺序固定：生产者"先写 buffer 再发布 head"；消费者"先读 head 再读 buffer 再发布 tail"。
 *
 * 容量模型：
 *   - 数组长度 RING_BUFFER_CAPACITY 必须为 2 的幂（_Static_assert）；
 *   - head == tail 表示空；保留一个空槽表示满；实际可用 RING_BUFFER_CAPACITY - 1 字节；
 *   - 索引通过 & (CAPACITY - 1) 回绕。
 *
 * 溢出策略（由上层决定）：RX 满时丢弃新字节并计数，不覆盖旧数据；
 * 本模块只负责"满则返回 false"，不做覆盖。
 */
#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RING_BUFFER_CAPACITY  512u   /* 数组长度（2 的幂）；实际可用 511 字节 */
_Static_assert((RING_BUFFER_CAPACITY & (RING_BUFFER_CAPACITY - 1u)) == 0u,
               "RING_BUFFER_CAPACITY 必须是 2 的幂");

/** SPSC 环形缓冲区实例（调用方静态分配） */
typedef struct {
    volatile uint32_t head;                  /* 生产者游标（生产者写，消费者只读） */
    volatile uint32_t tail;                  /* 消费者游标（消费者写，生产者只读） */
    uint8_t buffer[RING_BUFFER_CAPACITY];    /* 数据区 */
} ring_buffer_t;

/** 初始化（清空）。可在启动或 RX 重同步临界区内调用。 */
void  ring_buffer_init(ring_buffer_t *rb);

/** 入队一个字节。满则返回 false 且不写入。 */
bool  ring_buffer_push(ring_buffer_t *rb, uint8_t byte);

/** 出队一个字节。空则返回 false。 */
bool  ring_buffer_pop(ring_buffer_t *rb, uint8_t *byte);

/** 可读字节数（head - tail，按容量回绕）。 */
size_t ring_buffer_available(const ring_buffer_t *rb);

/** 剩余可写字节数（容量 - 1 - 可读数）。 */
size_t ring_buffer_free(const ring_buffer_t *rb);

/** 是否为空。 */
bool  ring_buffer_is_empty(const ring_buffer_t *rb);

/** 是否已满（仅剩一个空槽）。 */
bool  ring_buffer_is_full(const ring_buffer_t *rb);

/** 清空（仅用于重同步等临界区，正常读写不得并发调用）。 */
void  ring_buffer_reset(ring_buffer_t *rb);

#endif /* RING_BUFFER_H_ */
