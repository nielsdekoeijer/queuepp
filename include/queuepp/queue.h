#ifndef QUEUE_H
#define QUEUE_H

#include <stdatomic.h>
#include <atomic>
using namespace std;

extern "C" {

/*

                      Queue Description

  Queues work in index space.
   - You provide the buffer.
   - You mod the index by the buffer size.
     - This allows for arbitrary queue sizes while preserving
       power of two optimizations.
     - Use queue sizes of powers of two for optimal performance.


  In the diagram:
   'w' indicates entries that are being consumed.
   'x' indicates entries in the queue that are yet to be consumed.
   'p' indicates entries that are being pushed.
   ' ' indicates free space.

                  tail side
                   \     /
           committed|   |pending
                    |   |
          | | | | | |w|w|x|x|x|x|x|x|p|p| | | | | |
                                    |   |
                           committed|   |pending
                                    /   \
                                  head side

  The `committed` and `pending` values will only increment
  sequentially for both the head and tail sides.

  The diagram describes an MPMC queue. If the queue is single
  producer the `committed` and `pending` of the head will always
  be equal. If the queue is single consumer the same applies for
  the tail.

  Usage:
    Producer side:
        Use `prepare_push` functions to get an indices to work on.
        Use `commit_push` functions to commit to the queue

    Consumer side:
        Use `prepare_consume` functions to get an indices to work on.
        Use `commit_consume` functions to actually consume from the queue (frees indices)

*/

union MaybeAtomicU32 {
    atomic_uint atomic_value;
    unsigned int value;
};

struct QueueMultiSide {
    union MaybeAtomicU32 pending, committed;
};

union QueueSingleSide {
    union MaybeAtomicU32 pending, committed;
};

typedef struct SPSCQueue {
    union QueueSingleSide head;
    union QueueSingleSide tail;
    unsigned int queue_size; /* Should always be less than INT32_MAX */
    union {
        atomic_uint tail_waiters;
        atomic_uint head_waiters;
    };
} SPSCQueue;

// memset((queue), 0, sizeof((queue)[0]));
#define queue_init(queue, size)                             \
    {                                                       \
        *(queue) = (typeof(*queue)) { .queue_size = size }; \
    }
#define queue_get_used(queue) ((queue)->head.pending.atomic_value - (queue)->tail.committed.atomic_value)
#define queue_get_free(queue) ((int)((queue)->queue_size - queue_get_used(queue)))
#define queue_get_free_explicit(queue_size, head_pending, tail_committed) ((int)((queue_size) - ((head_pending) - (tail_committed))))
#define queue_get_committed(queue) ((queue)->head.committed.atomic_value - (queue)->tail.pending.atomic_value)
#define queue_get_committed_explicit(head_committed, tail_pending) ((head_committed) - (tail_pending))

/* Implementation */

#define uint unsigned int

void atomic_wake_one(atomic_uint*);
void atomic_wake_all(atomic_uint*);
void atomic_wait(atomic_uint* futex, uint expected_value);

// #define QUEUE_ATOMIC_WAKE(atomic_x, n)
#define QUEUE_ATOMIC_WAIT(atomic_x, v) atomic_wait(atomic_x, v)
#define QUEUE_ATOMIC_WAKE_ONE(atomic_x) atomic_wake_one(atomic_x)
#define QUEUE_ATOMIC_WAKE_ALL(atomic_x) atomic_wake_all(atomic_x)
#define QUEUE_ATOMIC_WAIT_AND_READ(atomic_x, v) \
    {                                           \
        QUEUE_ATOMIC_WAIT((atomic_x), (v));     \
        (v) = atomic_load(atomic_x);            \
    }
#define QUEUE_WAKE_TAIL_WAITER(tail_committed) QUEUE_ATOMIC_WAKE_ONE(tail_committed)
#define QUEUE_WAKE_HEAD_WAITER(head_committed) QUEUE_ATOMIC_WAKE_ONE(head_committed)
#define QUEUE_WAKE_ALL_TAIL_WAITERS(tail_committed) QUEUE_ATOMIC_WAKE_ALL(tail_committed)
#define QUEUE_WAKE_ALL_HEAD_WAITERS(head_committed) QUEUE_ATOMIC_WAKE_ALL(head_committed)

#define QUEUE_WAIT_FOR_TAIL(tail_waiters, tail_committed, v) \
    {                                                        \
        atomic_fetch_add(tail_waiters, 1);                   \
        QUEUE_ATOMIC_WAIT_AND_READ(tail_committed, v);       \
        atomic_fetch_sub(tail_waiters, 1);                   \
    }

#define QUEUE_WAIT_FOR_HEAD(head_waiters, head_committed, v) \
    {                                                        \
        atomic_fetch_add(head_waiters, 1);                   \
        QUEUE_ATOMIC_WAIT_AND_READ(head_committed, v);       \
        atomic_fetch_sub(head_waiters, 1);                   \
    }


/* --- Single producer implementation --- */

/* Returns an index to push or -1 on failure (Queue is full) */
static inline int sp_try_prepare_push(uint queue_size, uint head_pending, atomic_uint* tail_committed)
{
    if (queue_get_free_explicit(queue_size, head_pending, atomic_load(tail_committed)) <= 0)
        return -1;

    /* As this is an SP queue the usage cannot increase from this point
     * onwards, so we can safely return the current working index. */
    return head_pending;
}

/* Returns an index to push or -1 on failure (Queue is full) */
static inline int sp_try_prepare_push_many(uint queue_size, uint head_pending, atomic_uint* tail_committed, int count)
{
    if (queue_get_free_explicit(queue_size, head_pending, atomic_load(tail_committed)) < count)
        return -1;

    /* As this is an SP queue the usage cannot increase from this point
     * onwards, so we can safely return the current working index. */
    return head_pending;
}

/* Returns an index to push or -1 on failure (Queue is full) */
static inline int sp_prepare_push(uint queue_size, uint head_pending, atomic_uint* tail_committed, atomic_uint* tail_waiters)
{
    uint tail = atomic_load(tail_committed);

    while (queue_get_free_explicit(queue_size, head_pending, tail) <= 0)
        QUEUE_WAIT_FOR_TAIL(tail_waiters, tail_committed, tail);

    /* As this is an SP queue the usage cannot increase from this point
     * onwards, so we can safely return the current working index. */
    return head_pending;
}

/* Returns an index to push or -1 on failure (Queue is full) */
static inline int sp_prepare_push_many(uint queue_size, uint head_pending, atomic_uint* tail_committed, atomic_uint* tail_waiters, int count)
{
    uint tail = atomic_load(tail_committed);

    while (queue_get_free_explicit(queue_size, head_pending, tail) < count)
        QUEUE_WAIT_FOR_TAIL(tail_waiters, tail_committed, tail);

    /* As this is an SP queue the usage cannot increase from this point
     * onwards, so we can safely return the current working index. */
    return head_pending;
}

static inline void sp_commit_push(uint prepared_index, atomic_uint* head_committed, atomic_uint* head_waiters)
{
    atomic_fetch_add(head_committed, 1);

    if (atomic_load(head_waiters))
        QUEUE_WAKE_HEAD_WAITER(head_committed);
}

static inline void sp_commit_push_many(uint prepared_index, atomic_uint* head_committed, atomic_uint* head_waiters, int count)
{
    atomic_fetch_add(head_committed, count);

    if (atomic_load(head_waiters))
        QUEUE_WAKE_HEAD_WAITER(head_committed);
}


/* --- Single consumer implementation --- */

/* Returns an index to consume or -1 on failure (Queue is empty) */
static inline int sc_try_prepare_consume(atomic_uint* head_committed, uint tail_pending)
{
    if (queue_get_committed_explicit(atomic_load(head_committed), tail_pending) == 0)
        return -1;

    /* As this is an SC queue the committed cannot decrease from this point
     * onwards, so we can safely return the current working index. */
    return tail_pending;
}

/* Returns an index to consume or -1 on failure (Queue is empty) */
static inline int sc_try_prepare_consume_many(atomic_uint* head_committed, uint tail_pending, int count)
{
    if (queue_get_committed_explicit(atomic_load(head_committed), tail_pending) < count)
        return -1;

    /* As this is an SC queue the committed cannot decrease from this point
     * onwards, so we can safely return the current working index. */
    return tail_pending;
}

/* Returns an index to consume */
static inline int sc_prepare_consume(atomic_uint* head_committed, uint tail_pending, atomic_uint* head_waiters)
{
    uint head = atomic_load(head_committed);

    while (queue_get_committed_explicit(head, tail_pending) == 0)
        QUEUE_WAIT_FOR_HEAD(head_waiters, head_committed, head);

    /* As this is an SC queue the committed cannot decrease from this point
     * onwards, so we can consume safely. */
    return tail_pending;
}

/* Returns an index to consume */
static inline int sc_prepare_consume_many(atomic_uint* head_committed, uint tail_pending, atomic_uint* head_waiters, int count)
{
    uint head = atomic_load(head_committed);

    while (queue_get_committed_explicit(head, tail_pending) < count)
        QUEUE_WAIT_FOR_HEAD(head_waiters, head_committed, head);

    /* As this is an SC queue the committed cannot decrease from this point
     * onwards, so we can consume safely. */
    return tail_pending;
}

static inline void sc_commit_consume(uint prepared_index, atomic_uint* tail_committed, atomic_uint* tail_waiters)
{
    atomic_fetch_add(tail_committed, 1);

    /* As this is an SC queue any waiters would have to be the producer in this case */
    if (atomic_load(tail_waiters))
        QUEUE_WAKE_TAIL_WAITER(tail_committed);
}

static inline void sc_commit_consume_many(uint prepared_index, atomic_uint* tail_committed, atomic_uint* tail_waiters, int count)
{
    atomic_fetch_add(tail_committed, count);

    /* As this is an SC queue any waiters would have to be the producer in this case */
    if (atomic_load(tail_waiters))
        QUEUE_WAKE_TAIL_WAITER(tail_committed);
}


/* SPSC implementation */


/* Returns an index to push or -1 on failure (Queue is full) */
static inline int spsc_try_prepare_push(SPSCQueue* queue)
{
    return sp_try_prepare_push(queue->queue_size, queue->head.pending.value, &queue->tail.committed.atomic_value);
}

/* Returns an index to push or -1 on failure (Queue is full) */
static inline int spsc_prepare_push(SPSCQueue* queue)
{
    return sp_prepare_push(queue->queue_size, queue->head.pending.value,
        &queue->tail.committed.atomic_value, &queue->tail_waiters);
}

static inline void spsc_commit_push(unsigned int prepared_index, SPSCQueue* queue)
{
    sp_commit_push(prepared_index, &queue->head.committed.atomic_value, &queue->head_waiters);
}

/* Returns an index to consume or -1 on failure (Queue is empty) */
static inline int spsc_try_prepare_consume(SPSCQueue* queue)
{
    return sc_try_prepare_consume(&queue->head.committed.atomic_value, queue->tail.pending.value);
}

/* Returns an index to consume */
static inline int spsc_prepare_consume(SPSCQueue* queue)
{
    return sc_prepare_consume(&queue->head.committed.atomic_value, queue->tail.pending.value, &queue->head_waiters);
}

static inline void spsc_commit_consume(unsigned int prepared_index, SPSCQueue* queue)
{
    sc_commit_consume(prepared_index, &queue->tail.committed.atomic_value, &queue->tail_waiters);
}
}


#undef uint

#endif /* QUEUE_H */
