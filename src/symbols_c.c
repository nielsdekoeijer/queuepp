#include "queuepp/queue.h"

/* --- try_prepare_push --- */

__attribute__((used)) static inline int ref_spsc_try_prepare_push(void* data)
{
    SPSCQueue* queue = (SPSCQueue*)data;
    return spsc_try_prepare_push(queue);
}

/* --- prepare_push (blocking) --- */

__attribute__((used)) static inline int ref_spsc_prepare_push(void* data)
{
    SPSCQueue* queue = (SPSCQueue*)data;
    return spsc_prepare_push(queue);
}

/* --- commit_push --- */

__attribute__((used)) static inline void ref_spsc_commit_push(unsigned int prepared_index, void* data)
{
    SPSCQueue* queue = (SPSCQueue*)data;
    spsc_commit_push(prepared_index, queue);
}

/* --- try_prepare_consume --- */

__attribute__((used)) static inline int ref_spsc_try_prepare_consume(void* data)
{
    SPSCQueue* queue = (SPSCQueue*)data;
    return spsc_try_prepare_consume(queue);
}

/* --- prepare_consume (blocking) --- */

__attribute__((used)) static inline int ref_spsc_prepare_consume(void* data)
{
    SPSCQueue* queue = (SPSCQueue*)data;
    return spsc_prepare_consume(queue);
}

/* --- commit_consume --- */

__attribute__((used)) static inline void ref_spsc_commit_consume(unsigned int prepared_index, void* data)
{
    SPSCQueue* queue = (SPSCQueue*)data;
    spsc_commit_consume(prepared_index, queue);
}
