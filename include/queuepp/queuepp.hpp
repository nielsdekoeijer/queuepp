#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#define ALWAYS_INLINE __attribute__((always_inline)) inline

#define NEVER_INLINE __attribute__((noinline, cold))

namespace queuepp {

namespace detail {

/// Blocks the calling thread until *addr != expected (linux futex WAIT)
ALWAYS_INLINE void futex_wait(std::atomic<std::uint32_t>* addr, std::uint32_t expected)
{
    syscall(SYS_futex, addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected, nullptr, nullptr, 0);
}

/// Wakes one thread blocked on *addr (linux futex WAKE)
ALWAYS_INLINE void futex_wake(std::atomic<std::uint32_t>* addr)
{
    syscall(SYS_futex, addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1);
}

/// Represents the state for a `Single` Consumer / Producer queue.
///
/// Note that a union is used here for purposes of readability and to be able to more easily generalize to `Multi`
/// queue variants. Fields are uint32_t for direct futex compatibility.
union SingleSideQueue {
    std::atomic<std::uint32_t> pending, committed { 0 };

    SingleSideQueue() = default;
    SingleSideQueue(SingleSideQueue&) = delete;
    SingleSideQueue(SingleSideQueue&&) = delete;
    SingleSideQueue& operator=(SingleSideQueue&) = delete;
    SingleSideQueue& operator=(SingleSideQueue&&) = delete;
};


/// Represents the waiters for a `Single` Consumer / Producer queue.
///
/// For SPSC queues, tail and head waiters share the same memory location since the waiter count
/// is only used as a hint for "should I wake?". The actual wait happens on different futex
/// addresses (head.committed vs tail.committed), so sharing is safe — worst case is a spurious wake.
union SingleWaiters {
    std::atomic<std::uint32_t> tail, head { 0 };

    SingleWaiters() = default;
    SingleWaiters(SingleWaiters&) = delete;
    SingleWaiters(SingleWaiters&&) = delete;
    SingleWaiters& operator=(SingleWaiters&) = delete;
    SingleWaiters& operator=(SingleWaiters&&) = delete;
};

} // namespace detail

/// Provides indices for an underlying buffer for a SPSC lockfree queue
template <typename ElementType, std::size_t CacheSize = 128>
class SPSCQueueIndexer {
protected:
    alignas(CacheSize) detail::SingleSideQueue m_head;
    alignas(CacheSize) detail::SingleSideQueue m_tail;
    const std::uint32_t m_size;
    detail::SingleWaiters m_waiters;

public:
    explicit SPSCQueueIndexer(const std::uint32_t size)
        : m_size { size }
        , m_head {}
        , m_tail {}
        , m_waiters {}
    {
    }

    SPSCQueueIndexer() = delete;
    SPSCQueueIndexer(SPSCQueueIndexer&) = delete;
    SPSCQueueIndexer(SPSCQueueIndexer&&) = delete;
    SPSCQueueIndexer& operator=(SPSCQueueIndexer&) = delete;
    SPSCQueueIndexer& operator=(SPSCQueueIndexer&&) = delete;

protected:
    /// Returns how many entries are currently occupied (head - tail)
    std::uint32_t inline UsedCount(const std::uint32_t head, const std::uint32_t tail) const
    {
        return head - tail;
    }

    /// Returns how many entries can still be pushed (capacity - used)
    ALWAYS_INLINE std::uint32_t FreeCount(const std::uint32_t size, const std::uint32_t head, const std::uint32_t tail) const
    {
        return size - UsedCount(head, tail);
    }


    /// Blocks until the consumer commits, returns updated tail value
    NEVER_INLINE std::uint32_t WaitTail(std::uint32_t tail)
    {
        std::atomic_fetch_add_explicit(&m_waiters.tail, 1u, std::memory_order_relaxed);

        detail::futex_wait(&m_tail.committed, tail);
        tail = std::atomic_load_explicit(&m_tail.committed, std::memory_order_acquire);

        std::atomic_fetch_sub_explicit(&m_waiters.tail, 1u, std::memory_order_relaxed);
        return tail;
    }

    /// Wakes one producer blocked on tail; NEVER_INLINE keeps syscall clobbers off the fast path
    NEVER_INLINE void WakeTail()
    {
        detail::futex_wake(&m_tail.committed);
    }

    /// Blocks until the producer commits, returns updated head value
    NEVER_INLINE std::uint32_t WaitHead(std::uint32_t head)
    {
        std::atomic_fetch_add_explicit(&m_waiters.head, 1u, std::memory_order_relaxed);

        detail::futex_wait(&m_head.committed, head);
        head = std::atomic_load_explicit(&m_head.committed, std::memory_order_acquire);

        std::atomic_fetch_sub_explicit(&m_waiters.head, 1u, std::memory_order_relaxed);
        return head;
    }

    /// Wakes one consumer blocked on head; NEVER_INLINE keeps syscall clobbers off the fast path
    NEVER_INLINE void WakeHead()
    {
        detail::futex_wake(&m_head.committed);
    }

    enum class RuntimeMode {
        WAITFREE,
        BLOCKING,
    };

    enum class CountMode {
        SINGLE,
        MULTI,
    };

    template <RuntimeMode M>
    using PrepareReturnType = std::conditional_t<M == RuntimeMode::WAITFREE, std::optional<std::uint32_t>, std::uint32_t>;

    /// Reserves index(es) for pushing; blocks or returns nullopt depending on RuntimeMode
    template <RuntimeMode R, CountMode C>
    [[nodiscard]] PrepareReturnType<R> GenericPreparePush([[maybe_unused]] const std::uint32_t count = 1)
    {
        std::uint32_t c = 1;
        if constexpr (C == CountMode::MULTI) {
            c = count;
        }

        const std::uint32_t size = m_size;
        std::uint32_t tail = std::atomic_load_explicit(&m_tail.committed, std::memory_order_acquire);
        std::uint32_t head = std::atomic_load_explicit(&m_head.pending, std::memory_order_relaxed);

        [[unlikely]] while ((size - (head - tail)) < c) {
            if constexpr (R == RuntimeMode::WAITFREE) {
                return std::nullopt;
            } else {
                tail = WaitTail(tail);
            }
        }

        return head;
    }

    /// Publishes pushed entries and wakes a blocked consumer if any
    template <CountMode C>
    void GenericCommitPush([[maybe_unused]] const std::uint32_t count = 1)
    {
        std::uint32_t c = 1;
        if constexpr (C == CountMode::MULTI) {
            c = count;
        }

        std::atomic_fetch_add_explicit(&m_head.committed, c, std::memory_order_release);

        [[unlikely]] if (std::atomic_load_explicit(&m_waiters.head, std::memory_order_relaxed)) {
            WakeHead();
        }
    }

    /// Reserves index(es) for consuming; blocks or returns nullopt depending on RuntimeMode
    template <RuntimeMode R, CountMode C>
    [[nodiscard]] PrepareReturnType<R> GenericPrepareConsume([[maybe_unused]] const std::uint32_t count = 1)
    {
        std::uint32_t c = 1;
        if constexpr (C == CountMode::MULTI) {
            c = count;
        }

        std::uint32_t head = std::atomic_load_explicit(&m_head.committed, std::memory_order_acquire);
        std::uint32_t tail = std::atomic_load_explicit(&m_tail.pending, std::memory_order_relaxed);

        [[unlikely]] while (UsedCount(head, tail) < c) {
            if constexpr (R == RuntimeMode::WAITFREE) {
                return std::nullopt;
            } else {
                head = WaitHead(head);
            }
        }

        return tail;
    }

    /// Frees consumed entries and wakes a blocked producer if any
    template <CountMode C>
    void GenericCommitConsume([[maybe_unused]] const std::uint32_t count = 1)
    {
        std::uint32_t c = 1;
        if constexpr (C == CountMode::MULTI) {
            c = count;
        }

        std::atomic_fetch_add_explicit(&m_tail.committed, c, std::memory_order_release);

        [[unlikely]] if (std::atomic_load_explicit(&m_waiters.tail, std::memory_order_relaxed)) {
            WakeTail();
        }
    }

public:
    /// Non-blocking batch push reservation; returns start index or nullopt if insufficient space
    [[nodiscard]] inline std::optional<std::uint32_t> TryPreparePushMany(const std::uint32_t count)
    {
        return GenericPreparePush<RuntimeMode::WAITFREE, CountMode::MULTI>(count);
    }

    /// Blocking batch push reservation; waits until `count` slots are free
    [[nodiscard]] inline std::uint32_t PreparePushMany(const std::uint32_t count)
    {
        return GenericPreparePush<RuntimeMode::BLOCKING, CountMode::MULTI>(count);
    }

    /// Non-blocking single push reservation; returns index or nullopt if full
    [[nodiscard]] inline std::optional<std::uint32_t> TryPreparePush()
    {
        return GenericPreparePush<RuntimeMode::WAITFREE, CountMode::SINGLE>();
    }

    /// Blocking single push reservation; waits until a slot is free
    [[nodiscard]] inline std::uint32_t PreparePush()
    {
        return GenericPreparePush<RuntimeMode::BLOCKING, CountMode::SINGLE>();
    }

    /// Commits a single pushed entry, making it visible to the consumer
    inline void CommitPush()
    {
        GenericCommitPush<CountMode::SINGLE>();
    }

    /// Commits `count` pushed entries, making them visible to the consumer
    inline void CommitPushMany(const std::uint32_t count)
    {
        GenericCommitPush<CountMode::MULTI>(count);
    }

    /// Non-blocking batch consume reservation; returns start index or nullopt if insufficient entries
    [[nodiscard]] inline std::optional<std::uint32_t> TryPrepareConsumeMany(const std::uint32_t count)
    {
        return GenericPrepareConsume<RuntimeMode::WAITFREE, CountMode::MULTI>(count);
    }

    /// Blocking batch consume reservation; waits until `count` entries are available
    [[nodiscard]] inline std::uint32_t PrepareConsumeMany(const std::uint32_t count)
    {
        return GenericPrepareConsume<RuntimeMode::BLOCKING, CountMode::MULTI>(count);
    }

    /// Non-blocking single consume reservation; returns index or nullopt if empty
    [[nodiscard]] inline std::optional<std::uint32_t> TryPrepareConsume()
    {
        return GenericPrepareConsume<RuntimeMode::WAITFREE, CountMode::SINGLE>();
    }

    /// Blocking single consume reservation; waits until an entry is available
    [[nodiscard]] inline std::uint32_t PrepareConsume()
    {
        return GenericPrepareConsume<RuntimeMode::BLOCKING, CountMode::SINGLE>();
    }

    /// Commits a single consumed entry, freeing the slot for the producer
    inline void CommitConsume()
    {
        GenericCommitConsume<CountMode::SINGLE>();
    }

    /// Commits `count` consumed entries, freeing them for the producer
    inline void CommitConsumeMany(const std::uint32_t count)
    {
        GenericCommitConsume<CountMode::MULTI>(count);
    }
};

/// Non-owning SPSC lockfree queue
template <typename ElementType, std::size_t CacheSize = 128>
class SPSCQueueUnmanaged : public SPSCQueueIndexer<ElementType, CacheSize> {
protected:
    using Base = SPSCQueueIndexer<ElementType, CacheSize>;

    const std::uint32_t m_mask;

public:
    explicit SPSCQueueUnmanaged(const std::uint32_t size)
        : Base(size)
        , m_mask(size - 1)
    {
        if (size == 0 || (size & (size - 1)) != 0) {
            throw std::runtime_error("SPSQueueUnmanaged only supports radix-2 sized backers");
        }
    }

    SPSCQueueUnmanaged() = delete;
    SPSCQueueUnmanaged(SPSCQueueUnmanaged&) = delete;
    SPSCQueueUnmanaged(SPSCQueueUnmanaged&&) = delete;
    SPSCQueueUnmanaged& operator=(SPSCQueueUnmanaged&) = delete;
    SPSCQueueUnmanaged& operator=(SPSCQueueUnmanaged&&) = delete;

protected:
    template <Base::RuntimeMode R>
    using ReturnType = std::conditional_t<R == Base::RuntimeMode::WAITFREE, bool, void>;


    /// Copies `count` elements into the ring buffer, handling wraparound
    template <typename Backing, typename Source>
    void CopyIn(Backing& backing, const std::uint32_t head, const Source& src, const std::uint32_t count)
    {
        const std::uint32_t start = head & m_mask;
        const std::uint32_t first = std::min<std::uint32_t>(count, this->m_size - start);

        // Semantically:
        // for (std::uint32_t i = 0; i < first; ++i) {
        //     backing[start + i] = src[i];
        // }
        //
        // for (std::uint32_t i = first; i < count; ++i) {
        //     backing[i - first] = src[i];
        // }

        const auto* s = &src[0];
        auto* b = &backing[0];
        std::copy_n(s, first, b + start);
        std::copy_n(s + first, count - first, b);
    }

    /// Copies `count` elements out of the ring buffer, handling wraparound
    template <typename Backing, typename Destination>
    void CopyOut(const Backing& backing, const std::uint32_t tail, Destination& dst, const std::uint32_t count)
    {
        const std::uint32_t start = tail & m_mask;
        const std::uint32_t first = std::min<std::uint32_t>(count, this->m_size - start);

        // Semantically:
        // for (std::uint32_t i = 0; i < first; ++i) {
        //     dst[i] = backing[start + i];
        // }
        //
        // for (std::uint32_t i = first; i < count; ++i) {
        //     dst[i] = backing[i - first];
        // }

        const auto* b = &backing[0];
        auto* d = &dst[0];
        std::copy_n(b + start, first, d);
        std::copy_n(b, count - first, d + first);
    }

    /// Prepares, copies in, and commits a push in one operation
    template <Base::RuntimeMode R, Base::CountMode C, typename Backing, typename Source>
    ReturnType<R> GenericPush(Backing& backing, const Source& src, [[maybe_unused]] const std::uint32_t count = 1)
    {
        const std::uint32_t c = (C == Base::CountMode::SINGLE) ? std::uint32_t { 1 } : count;

        const auto head = this->template GenericPreparePush<R, C>(count);

        if constexpr (R == Base::RuntimeMode::WAITFREE) {
            [[unlikely]] if (!head) {
                return false;
            }

            CopyIn(backing, *head, src, c);
            this->template GenericCommitPush<C>(count);
            return true;
        } else {
            CopyIn(backing, head, src, c);
            this->template GenericCommitPush<C>(count);
        }
    }

    /// Prepares, copies out, and commits a consume in one operation
    template <Base::RuntimeMode R, Base::CountMode C, typename Backing, typename Destination>
    ReturnType<R> GenericPop(const Backing& backing, Destination& dst, [[maybe_unused]] const std::uint32_t count = 1)
    {
        const std::uint32_t c = (C == Base::CountMode::SINGLE) ? std::uint32_t { 1 } : count;

        const auto tail = this->template GenericPrepareConsume<R, C>(count);

        if constexpr (R == Base::RuntimeMode::WAITFREE) {
            [[unlikely]] if (!tail) {
                return false;
            }

            CopyOut(backing, *tail, dst, c);
            this->template GenericCommitConsume<C>(count);
            return true;
        } else {
            CopyOut(backing, tail, dst, c);
            this->template GenericCommitConsume<C>(count);
        }
    }

public:
    /// Non-blocking push of one element; returns false if full
    template <typename Backing, typename Source>
    [[nodiscard]] bool TryPush(Backing& backing, const Source& src)
    {
        return GenericPush<Base::RuntimeMode::WAITFREE, Base::CountMode::SINGLE>(backing, src);
    }

    /// Blocking push of one element; waits until space is available
    template <typename Backing, typename Source>
    void Push(Backing& backing, const Source& src)
    {
        GenericPush<Base::RuntimeMode::BLOCKING, Base::CountMode::SINGLE>(backing, src);
    }

    /// Non-blocking batch push; returns false if insufficient space
    template <typename Backing, typename Source>
    [[nodiscard]] bool TryPushMany(Backing& backing, const Source& src, const std::uint32_t count)
    {
        return GenericPush<Base::RuntimeMode::WAITFREE, Base::CountMode::MULTI>(backing, src, count);
    }

    /// Blocking batch push; waits until `count` slots are free
    template <typename Backing, typename Source>
    void PushMany(Backing& backing, const Source& src, const std::uint32_t count)
    {
        GenericPush<Base::RuntimeMode::BLOCKING, Base::CountMode::MULTI>(backing, src, count);
    }

    /// Non-blocking pop of one element; returns false if empty
    template <typename Backing, typename Destination>
    [[nodiscard]] bool TryPop(const Backing& backing, Destination& dst)
    {
        return GenericPop<Base::RuntimeMode::WAITFREE, Base::CountMode::SINGLE>(backing, dst);
    }

    /// Blocking pop of one element; waits until an entry is available
    template <typename Backing, typename Destination>
    void Pop(const Backing& backing, Destination& dst)
    {
        GenericPop<Base::RuntimeMode::BLOCKING, Base::CountMode::SINGLE>(backing, dst);
    }

    /// Non-blocking batch pop; returns false if insufficient entries
    template <typename Backing, typename Destination>
    [[nodiscard]] bool TryPopMany(const Backing& backing, Destination& dst, const std::uint32_t count)
    {
        return GenericPop<Base::RuntimeMode::WAITFREE, Base::CountMode::MULTI>(backing, dst, count);
    }

    /// Blocking batch pop; waits until `count` entries are available
    template <typename Backing, typename Destination>
    void PopMany(const Backing& backing, Destination& dst, const std::uint32_t count)
    {
        GenericPop<Base::RuntimeMode::BLOCKING, Base::CountMode::MULTI>(backing, dst, count);
    }
};

} // namespace queuepp
