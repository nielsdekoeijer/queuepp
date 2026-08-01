#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>


namespace queuepp {

namespace detail {
/// Represents a `Single` Consumer / Producer for our queue
template <typename UnsignedType>
union SingleSideQueue {
    static_assert(std::atomic<UnsignedType>::is_always_lock_free);
    std::atomic<UnsignedType> pending, committed { 0 };

    SingleSideQueue() = default;
    SingleSideQueue(SingleSideQueue&) = delete;
    SingleSideQueue(SingleSideQueue&&) = delete;
    SingleSideQueue& operator=(SingleSideQueue&) = delete;
    SingleSideQueue& operator=(SingleSideQueue&&) = delete;
};


/// Represents a `Single` Waiter for our queue
template <typename UnsignedType>
union SingleWaiters {
    static_assert(std::atomic<UnsignedType>::is_always_lock_free);
    std::atomic<UnsignedType> tail, head { 0 };

    SingleWaiters() = default;
    SingleWaiters(SingleWaiters&) = delete;
    SingleWaiters(SingleWaiters&&) = delete;
    SingleWaiters& operator=(SingleWaiters&) = delete;
    SingleWaiters& operator=(SingleWaiters&&) = delete;
};

} // namespace detail

/// SPSC lockfree queue
template < //
    typename ElementType,
    typename UnsignedType = std::uint64_t,
    std::size_t CacheSize = 128>
class SPSCQueueIndexer {
protected:
    const UnsignedType m_size;
    alignas(CacheSize) detail::SingleSideQueue<UnsignedType> m_head;
    alignas(CacheSize) detail::SingleSideQueue<UnsignedType> m_tail;
    alignas(CacheSize) detail::SingleWaiters<UnsignedType> m_waiters;

public:
    explicit SPSCQueueIndexer(const UnsignedType size)
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
    UnsignedType inline FreeCount(const UnsignedType head, const UnsignedType tail) const
    {
        return m_size - (head - tail);
    }

    UnsignedType inline UsedCount(const UnsignedType head, const UnsignedType tail) const
    {
        return head - tail;
    }

    inline void WaitTail(UnsignedType& tail)
    {
        std::atomic_fetch_add_explicit(&m_waiters.tail, 1, std::memory_order_seq_cst);

        std::atomic_wait_explicit(&m_tail.committed, tail, std::memory_order_seq_cst);
        tail = std::atomic_load_explicit(&m_tail.committed, std::memory_order_seq_cst);

        std::atomic_fetch_sub_explicit(&m_waiters.tail, 1, std::memory_order_seq_cst);
    }

    inline void WaitHead(UnsignedType& head)
    {
        std::atomic_fetch_add_explicit(&m_waiters.head, 1, std::memory_order_seq_cst);

        std::atomic_wait_explicit(&m_head.committed, head, std::memory_order_seq_cst);
        head = std::atomic_load_explicit(&m_head.committed, std::memory_order_seq_cst);

        std::atomic_fetch_sub_explicit(&m_waiters.head, 1, std::memory_order_seq_cst);
    }

    inline void WakeTail()
    {
        std::atomic_notify_one(&m_tail.committed);
    }

    inline void WakeHead()
    {
        std::atomic_notify_one(&m_head.committed);
    }

    enum class RuntimeMode {
        TRY,
        BLOCK,
    };

    enum class CountMode {
        SINGLE,
        MANY,
    };

    template <RuntimeMode M>
    using PrepareReturnType = std::conditional_t<M == RuntimeMode::TRY, std::optional<UnsignedType>, UnsignedType>;

    template <RuntimeMode R, CountMode C>
    [[nodiscard]] PrepareReturnType<R> GenericPreparePush([[maybe_unused]] const UnsignedType count = 1)
    {
        UnsignedType c;
        if constexpr (C == CountMode::SINGLE) {
            c = 1;
        } else {
            c = count;
        }

        UnsignedType tail = std::atomic_load_explicit(&m_tail.committed, std::memory_order_seq_cst);

        UnsignedType head = std::atomic_load_explicit(&m_head.pending, std::memory_order_relaxed);

        while (FreeCount(head, tail) < c) {
            if constexpr (R == RuntimeMode::TRY) {
                return std::nullopt;
            } else {
                WaitTail(tail);
            }
        }

        return head;
    }

    template <CountMode C>
    void GenericCommitPush([[maybe_unused]] const UnsignedType count = 1)
    {
        UnsignedType c;
        if constexpr (C == CountMode::SINGLE) {
            c = 1;
        } else {
            c = count;
        }

        std::atomic_fetch_add_explicit(&m_head.committed, c, std::memory_order_seq_cst);

        [[unlikely]] if (std::atomic_load_explicit(&m_waiters.head, std::memory_order_seq_cst)) {
            WakeHead();
        }
    }

    template <RuntimeMode R, CountMode C>
    [[nodiscard]] PrepareReturnType<R> GenericPrepareConsume([[maybe_unused]] const UnsignedType count = 1)
    {
        UnsignedType c;
        if constexpr (C == CountMode::SINGLE) {
            c = 1;
        } else {
            c = count;
        }

        UnsignedType head = std::atomic_load_explicit(&m_head.committed, std::memory_order_seq_cst);

        UnsignedType tail = std::atomic_load_explicit(&m_tail.pending, std::memory_order_relaxed);

        while (UsedCount(head, tail) < c) {
            if constexpr (R == RuntimeMode::TRY) {
                return std::nullopt;
            } else {
                WaitHead(head);
            }
        }

        return tail;
    }

    template <CountMode C>
    void GenericCommitConsume([[maybe_unused]] const UnsignedType count = 1)
    {
        UnsignedType c;
        if constexpr (C == CountMode::SINGLE) {
            c = 1;
        } else {
            c = count;
        }

        std::atomic_fetch_add_explicit(&m_tail.committed, c, std::memory_order_seq_cst);

        [[unlikely]] if (std::atomic_load_explicit(&m_waiters.tail, std::memory_order_seq_cst)) {
            WakeTail();
        }
    }

public:
    [[nodiscard]] inline std::optional<UnsignedType> TryPreparePushMany(const UnsignedType count)
    {
        return GenericPreparePush<RuntimeMode::TRY, CountMode::MANY>(count);
    }

    [[nodiscard]] inline UnsignedType PreparePushMany(const UnsignedType count)
    {
        return GenericPreparePush<RuntimeMode::BLOCK, CountMode::MANY>(count);
    }

    [[nodiscard]] inline std::optional<UnsignedType> TryPreparePush()
    {
        return GenericPreparePush<RuntimeMode::TRY, CountMode::SINGLE>();
    }

    [[nodiscard]] inline UnsignedType PreparePush()
    {
        return GenericPreparePush<RuntimeMode::BLOCK, CountMode::SINGLE>();
    }

    inline void CommitPush()
    {
        GenericCommitPush<CountMode::SINGLE>();
    }

    inline void CommitPushMany(const UnsignedType count)
    {
        GenericCommitPush<CountMode::MANY>(count);
    }

    [[nodiscard]] inline std::optional<UnsignedType> TryPrepareConsumeMany(const UnsignedType count)
    {
        return GenericPrepareConsume<RuntimeMode::TRY, CountMode::MANY>(count);
    }

    [[nodiscard]] inline UnsignedType PrepareConsumeMany(const UnsignedType count)
    {
        return GenericPrepareConsume<RuntimeMode::BLOCK, CountMode::MANY>(count);
    }

    [[nodiscard]] inline std::optional<UnsignedType> TryPrepareConsume()
    {
        return GenericPrepareConsume<RuntimeMode::TRY, CountMode::SINGLE>();
    }

    [[nodiscard]] inline UnsignedType PrepareConsume()
    {
        return GenericPrepareConsume<RuntimeMode::BLOCK, CountMode::SINGLE>();
    }

    inline void CommitConsume()
    {
        GenericCommitConsume<CountMode::SINGLE>();
    }

    inline void CommitConsumeMany(const UnsignedType count)
    {
        GenericCommitConsume<CountMode::MANY>(count);
    }
};

template < //
    typename ElementType,
    typename UnsignedType = std::uint64_t,
    std::size_t CacheSize = 128>
class SPSCQueueUnmanaged : public SPSCQueueIndexer<ElementType, UnsignedType, CacheSize> {
protected:
    using Base = SPSCQueueIndexer<ElementType, UnsignedType, CacheSize>;

    const UnsignedType m_mask;

public:
    explicit SPSCQueueUnmanaged(const UnsignedType size)
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
    using ReturnType = std::conditional_t<R == Base::RuntimeMode::TRY, bool, void>;


    template <typename Backing, typename Source>
    void CopyIn(Backing& backing, const UnsignedType head, const Source& src, const UnsignedType count)
    {
        const UnsignedType start = head & m_mask;
        const UnsignedType first = std::min<UnsignedType>(count, this->m_size - start);

        // Semantically:
        // for (UnsignedType i = 0; i < first; ++i) {
        //     backing[start + i] = src[i];
        // }
        //
        // for (UnsignedType i = first; i < count; ++i) {
        //     backing[i - first] = src[i];
        // }

        const auto* s = &src[0];
        auto* b = &backing[0];
        std::copy_n(s, first, b + start);
        std::copy_n(s + first, count - first, b);
    }

    template <typename Backing, typename Destination>
    void CopyOut(const Backing& backing, const UnsignedType tail, Destination& dst, const UnsignedType count)
    {
        const UnsignedType start = tail & m_mask;
        const UnsignedType first = std::min<UnsignedType>(count, this->m_size - start);

        // Semantically:
        // for (UnsignedType i = 0; i < first; ++i) {
        //     dst[i] = backing[start + i];
        // }
        //
        // for (UnsignedType i = first; i < count; ++i) {
        //     dst[i] = backing[i - first];
        // }

        const auto* b = &backing[0];
        auto* d = &dst[0];
        std::copy_n(b + start, first, d);
        std::copy_n(b, count - first, d + first);
    }

    template <Base::RuntimeMode R, Base::CountMode C, typename Backing, typename Source>
    ReturnType<R> GenericPush(Backing& backing, const Source& src, [[maybe_unused]] const UnsignedType count = 1)
    {
        const UnsignedType c = (C == Base::CountMode::SINGLE) ? UnsignedType { 1 } : count;

        const auto head = this->template GenericPreparePush<R, C>(count);

        if constexpr (R == Base::RuntimeMode::TRY) {
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

    template <Base::RuntimeMode R, Base::CountMode C, typename Backing, typename Destination>
    ReturnType<R> GenericPop(const Backing& backing, Destination& dst, [[maybe_unused]] const UnsignedType count = 1)
    {
        const UnsignedType c = (C == Base::CountMode::SINGLE) ? UnsignedType { 1 } : count;

        const auto tail = this->template GenericPrepareConsume<R, C>(count);

        if constexpr (R == Base::RuntimeMode::TRY) {
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
    template <typename Backing, typename Source>
    [[nodiscard]] bool TryPush(Backing& backing, const Source& src)
    {
        return GenericPush<Base::RuntimeMode::TRY, Base::CountMode::SINGLE>(backing, src);
    }

    template <typename Backing, typename Source>
    void Push(Backing& backing, const Source& src)
    {
        GenericPush<Base::RuntimeMode::BLOCK, Base::CountMode::SINGLE>(backing, src);
    }

    template <typename Backing, typename Source>
    [[nodiscard]] bool TryPushMany(Backing& backing, const Source& src, const UnsignedType count)
    {
        return GenericPush<Base::RuntimeMode::TRY, Base::CountMode::MANY>(backing, src, count);
    }

    template <typename Backing, typename Source>
    void PushMany(Backing& backing, const Source& src, const UnsignedType count)
    {
        GenericPush<Base::RuntimeMode::BLOCK, Base::CountMode::MANY>(backing, src, count);
    }

    template <typename Backing, typename Destination>
    [[nodiscard]] bool TryPop(const Backing& backing, Destination& dst)
    {
        return GenericPop<Base::RuntimeMode::TRY, Base::CountMode::SINGLE>(backing, dst);
    }

    template <typename Backing, typename Destination>
    void Pop(const Backing& backing, Destination& dst)
    {
        GenericPop<Base::RuntimeMode::BLOCK, Base::CountMode::SINGLE>(backing, dst);
    }

    template <typename Backing, typename Destination>
    [[nodiscard]] bool TryPopMany(const Backing& backing, Destination& dst, const UnsignedType count)
    {
        return GenericPop<Base::RuntimeMode::TRY, Base::CountMode::MANY>(backing, dst, count);
    }

    template <typename Backing, typename Destination>
    void PopMany(const Backing& backing, Destination& dst, const UnsignedType count)
    {
        GenericPop<Base::RuntimeMode::BLOCK, Base::CountMode::MANY>(backing, dst, count);
    }
};

} // namespace queuepp
