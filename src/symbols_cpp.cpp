#include "queuepp/queuepp.hpp"

using Indexer = queuepp::SPSCQueueIndexer<float, 4>;

extern "C" {

/* --- try_prepare_push --- */

__attribute__((used)) inline int new_spsc_try_prepare_push(void* data)
{
    auto queue = reinterpret_cast<Indexer*>(data);
    auto result = queue->TryPreparePush();
    return result ? (int)*result : -1;
}

/* --- prepare_push (blocking) --- */

__attribute__((used)) inline int new_spsc_prepare_push(void* data)
{
    auto queue = reinterpret_cast<Indexer*>(data);
    return (int)queue->PreparePush();
}

/* --- commit_push --- */

__attribute__((used)) inline void new_spsc_commit_push([[maybe_unused]] unsigned int prepared_index, void* data)
{
    auto queue = reinterpret_cast<Indexer*>(data);
    queue->CommitPush();
}

/* --- try_prepare_consume --- */

__attribute__((used)) inline int new_spsc_try_prepare_consume(void* data)
{
    auto queue = reinterpret_cast<Indexer*>(data);
    auto result = queue->TryPrepareConsume();
    return result ? (int)*result : -1;
}

/* --- prepare_consume (blocking) --- */

__attribute__((used)) inline int new_spsc_prepare_consume(void* data)
{
    auto queue = reinterpret_cast<Indexer*>(data);
    return (int)queue->PrepareConsume();
}

/* --- commit_consume --- */

__attribute__((used)) inline void new_spsc_commit_consume([[maybe_unused]] unsigned int prepared_index, void* data)
{
    auto queue = reinterpret_cast<Indexer*>(data);
    queue->CommitConsume();
}

}
