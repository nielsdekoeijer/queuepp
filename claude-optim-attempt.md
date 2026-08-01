# Making `queuepp` fast

Every change below is in `queuepp.hpp`. The starting point measured **37,688
ops/ms** throughput and **644 ns** round-trip latency; the end point measures
**~500,000 ops/ms** and **~125 ns** (pinned to two separate physical cores).

The gap to the lock-free queues came from three things, in order of impact:

1. reading the *other* thread's atomic on **every** operation (a guaranteed
   cross-core cache miss per op),
2. `seq_cst` on loads that only needed `acquire`/`relaxed`,
3. dropping straight into a futex `atomic_wait` instead of spinning first.

Each is addressed below with the before/after code.

---

## How the cost was located

Before touching the queue, a 4-variant microbenchmark toggled the two suspected
axes independently (memory order × index caching) on a minimal ring with the
same structure. It reproduced the real numbers and isolated the causes:

| Variant | ops/ms | ns/op |
| --- | ---: | ---: |
| `seq_cst` + `fetch_add`, no cache **(≈ original queuepp)** | 36,377 | 27.5 |
| `acq/rel` + plain store, no cache | 315,886 | 3.17 |
| `seq_cst` + `fetch_add`, cached | 188,011 | 5.32 |
| `acq/rel` + plain store, cached **(≈ dro)** | 1,506,296 | 0.66 |

Caching alone is ~5×; relaxing the barriers is another large factor; they
compound. The harness itself was fair — the cost was entirely in the queue.

---

## Change 1 — Cache the other side's index (the big one)

**Problem.** Every `Prepare*` read the *other* thread's `committed` atomic, so
each op paid a cross-core coherence miss.

```cpp
// BEFORE — GenericPreparePush: reads the consumer's atomic on EVERY push
UnsignedType tail = std::atomic_load_explicit(&m_tail.committed, std::memory_order_seq_cst);
UnsignedType head = std::atomic_load_explicit(&m_head.pending,   std::memory_order_relaxed);

while (FreeCount(head, tail) < c) {
    if constexpr (R == RuntimeMode::TRY) { return std::nullopt; }
    else { WaitTail(tail); }
}
return head;
```

**Fix.** Keep a private, lagging copy of the other side's index and only reload
the shared atomic when the copy says the queue looks full/empty. New members,
each placed on the line that its owner already dirties every op (so co-locating
the private cache there is free):

```cpp
// BEFORE
const UnsignedType m_size;
alignas(CacheSize) detail::SingleSideQueue<UnsignedType> m_head;
alignas(CacheSize) detail::SingleSideQueue<UnsignedType> m_tail;
alignas(CacheSize) detail::SingleWaiters<UnsignedType>   m_waiters;

// AFTER
const UnsignedType m_size;
alignas(CacheSize) detail::SingleSideQueue<UnsignedType> m_head;
UnsignedType m_tailCache;   // producer's view of the committed tail (on m_head's line)
alignas(CacheSize) detail::SingleSideQueue<UnsignedType> m_tail;
UnsignedType m_headCache;   // consumer's view of the committed head (on m_tail's line)
alignas(CacheSize) detail::SingleWaiters<UnsignedType>   m_waiters;
```

```cpp
// constructor: initialise both caches to 0
: m_size { size }, m_head {}, m_tailCache { 0 }, m_tail {}, m_headCache { 0 }, m_waiters {}
```

```cpp
// AFTER — GenericPreparePush: the fast path never touches the consumer's line
const UnsignedType head = std::atomic_load_explicit(&m_head.pending, std::memory_order_relaxed);

// Cache lags the real tail, so it only ever UNDER-estimates free space -> safe.
if (FreeCount(head, m_tailCache) >= c) {
    return head;
}

// Only now do we read the shared atomic, and remember it.
UnsignedType tail = std::atomic_load_explicit(&m_tail.committed, std::memory_order_acquire);
m_tailCache = tail;

while (FreeCount(head, tail) < c) {
    if constexpr (R == RuntimeMode::TRY) { return std::nullopt; }
    else { WaitTail(tail); m_tailCache = tail; }
}
return head;
```

`GenericPrepareConsume` gets the mirror treatment with `m_headCache`:

```cpp
// AFTER — GenericPrepareConsume
const UnsignedType tail = std::atomic_load_explicit(&m_tail.pending, std::memory_order_relaxed);

// Data written before the producer's release store is visible through the
// acquire load that last refreshed the cache, so trusting the cache is safe.
if (UsedCount(m_headCache, tail) >= c) {
    return tail;
}

UnsignedType head = std::atomic_load_explicit(&m_head.committed, std::memory_order_acquire);
m_headCache = head;

while (UsedCount(head, tail) < c) {
    if constexpr (R == RuntimeMode::TRY) { return std::nullopt; }
    else { WaitHead(head); m_headCache = head; }
}
return tail;
```

**Why it's correct:** the cache always *lags* the true index, so it can only
under-report available space/items — never enough to overfill or over-read. The
happens-before for the data is carried transitively by the `acquire` load that
last refreshed the cache.

---

## Change 2 — Relax the memory orders that were needlessly `seq_cst`

The `Prepare*` loads of the other side went from `seq_cst` to `acquire` (see the
`acquire` loads above). On the commit side, only the **waiter-flag load** could
be relaxed; the publication itself must stay a `seq_cst` RMW (Change 3 explains
why).

```cpp
// BEFORE — GenericCommitPush
std::atomic_fetch_add_explicit(&m_head.committed, c, std::memory_order_seq_cst);
[[unlikely]] if (std::atomic_load_explicit(&m_waiters.head, std::memory_order_seq_cst)) {
    WakeHead();
}

// AFTER — waiter load relaxed; the seq_cst RMW already orders it
std::atomic_fetch_add_explicit(&m_head.committed, c, std::memory_order_seq_cst);
[[unlikely]] if (std::atomic_load_explicit(&m_waiters.head, std::memory_order_relaxed)) {
    WakeHead();
}
```

`GenericCommitConsume` gets the same one-word change (`m_waiters.tail` load
→ `relaxed`).

> A tried-and-reverted step: replacing the commit `fetch_add(seq_cst)` with a
> `release` store + standalone `seq_cst` fence. It measured **~9 % worse RTT**
> (700 vs 638 ns) — the fence is still mandatory (Change 3), so it only added
> instructions. The fused RMW is the right primitive; that change was dropped.

---

## Change 3 — Spin before you sleep (the latency win)

**Problem.** `queuepp` is a *blocking* queue: when empty/full it calls a futex
`atomic_wait`. In a ping-pong workload the queue is empty every exchange, so it
parked and woke on every single message — 644 ns round trips, ~5× the spinning
queues.

```cpp
// BEFORE — WaitTail: register as a waiter and immediately park on the futex
inline void WaitTail(UnsignedType& tail)
{
    std::atomic_fetch_add_explicit(&m_waiters.tail, 1, std::memory_order_seq_cst);
    std::atomic_wait_explicit(&m_tail.committed, tail, std::memory_order_seq_cst);
    tail = std::atomic_load_explicit(&m_tail.committed, std::memory_order_seq_cst);
    std::atomic_fetch_sub_explicit(&m_waiters.tail, 1, std::memory_order_seq_cst);
}
```

**Fix.** Spin a bounded number of times first. When the peer is running on
another core it almost always advances within that window, so an active pair
never enters the kernel; only a genuinely idle waiter pays for the syscall.

A portable pause hint (added to the `detail` namespace):

```cpp
inline void cpuRelax() noexcept
{
#if defined(__x86_64__) || defined(__i386__)
    __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(__arm__)
    asm volatile("yield" ::: "memory");
#else
    std::atomic_signal_fence(std::memory_order_seq_cst);
#endif
}
```

```cpp
// AFTER — WaitTail: spin-then-block
static constexpr std::size_t kSpinCount = 1000;

inline void WaitTail(UnsignedType& tail)
{
    // Fast path: spin. The consumer commits with a seq_cst RMW, so an acquire
    // load observes it promptly, without touching m_waiters.
    for (std::size_t spin = 0; spin < kSpinCount; ++spin) {
        const UnsignedType t = std::atomic_load_explicit(&m_tail.committed, std::memory_order_acquire);
        if (t != tail) { tail = t; return; }
        detail::cpuRelax();
    }

    // Slow path: register, then RE-CHECK committed before parking so a commit
    // published during the spin cannot be missed (the waker's seq_cst RMW
    // orders its waiter load after this registration).
    std::atomic_fetch_add_explicit(&m_waiters.tail, 1, std::memory_order_seq_cst);
    UnsignedType t = std::atomic_load_explicit(&m_tail.committed, std::memory_order_seq_cst);
    while (t == tail) {
        std::atomic_wait_explicit(&m_tail.committed, tail, std::memory_order_seq_cst);
        t = std::atomic_load_explicit(&m_tail.committed, std::memory_order_seq_cst);
    }
    tail = t;
    std::atomic_fetch_sub_explicit(&m_waiters.tail, 1, std::memory_order_seq_cst);
}
```

`WaitHead` is the exact mirror on `m_head.committed` / `m_waiters.head`.

**Why the commit RMW must stay `seq_cst` (the lost-wakeup rule).** A consumer
about to sleep does: bump `m_waiters`, then re-read `committed`. The producer
does: publish `committed`, then read `m_waiters`. If either side's store can be
reordered past its load, the producer can read "no waiters" while the consumer
reads the stale index and sleeps forever. The `seq_cst` RMW on the commit is the
StoreLoad barrier that forbids exactly that interleaving — so it cannot be
relaxed. The spin doesn't change this: it only avoids *reaching* the slow path.

---

## Result

| Metric | Before | After | Change |
| --- | ---: | ---: | ---: |
| Throughput | 37,688 ops/ms | ~500,000 ops/ms | **~13×** |
| Round-trip | 644 ns | ~125 ns | **~5×** |

After this, `queuepp` beats boost/folly/rigtorp on throughput, matches
moodycamel, and leads every queue on RTT (including dro at ~160 ns). It still
trails `dro` on pure streaming throughput (~500K vs ~1.19M) because `dro`'s
commit is a plain `release` store with no waiter bookkeeping — the price of
`queuepp`'s blocking (futex) semantics, which the RTT number pays back.

**Verification.** A stress test drives tiny rings that force the full/empty
park/wake path (with millisecond peer delays so the waiter genuinely sleeps),
and passes under both `-O3` and ThreadSanitizer — no deadlock, no data races,
values received in order.
