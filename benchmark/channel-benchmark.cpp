// Compares Opal::ChannelSPSC, Opal::ChannelMPMC and Opal::ChannelMPSC, in both their signaling and spinning forms, against a
// bounded queue guarded by std::mutex and std::condition_variable, the queue a codebase without a lock-free one would write.
//
// Two operations:
//   FillDrain - one thread fills the channel with TrySend, then empties it with TryReceive. No contention, so it is the cost of
//               an operation on its own, with the buffer swept from L1 out to L2.
//   Transfer  - producer threads Send a fixed number of items while consumer threads Receive them until the channel is closed.
//               The cost of handing items between cores, including waiting on a full or empty channel.
//
// Every benchmark body is written once as a template over the channel type, so all sides do exactly the same work.
//
// See docs/benchmarks.md for how to build, run and compare the results.

#include <benchmark/benchmark.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "opal/threading/channel-mpmc.h"
#include "opal/threading/channel-mpsc.h"
#include "opal/threading/channel-spsc.h"
#include "opal/threading/thread.h"

#include "thread-placement.h"

namespace
{

using namespace Bench;

using Item = Opal::u64;

using SpscSignal = Opal::ChannelSPSC<Item, true>;
using SpscSpin = Opal::ChannelSPSC<Item, false>;
using MpmcSignal = Opal::ChannelMPMC<Item, true>;
using MpmcSpin = Opal::ChannelMPMC<Item, false>;
using MpscSignal = Opal::ChannelMPSC<Item, true>;
using MpscSpin = Opal::ChannelMPSC<Item, false>;

// Items handed over per Transfer iteration, split evenly between the producers.
constexpr size_t k_transfer_items = size_t{1} << 18;

/**
 * Bounded ring buffer guarded by one mutex, with condition variables to wait on while it is full or empty. Pop reports false once
 * the queue is closed and drained, matching Receive on the Opal channels.
 */
template <typename T>
class MutexQueue
{
public:
    explicit MutexQueue(size_t capacity) : m_items(capacity) {}

    void Push(const T& item)
    {
        std::unique_lock lock(m_mutex);
        m_not_full.wait(lock, [this] { return m_size < m_items.size(); });
        PushLocked(item);
        lock.unlock();
        m_not_empty.notify_one();
    }

    bool TryPush(const T& item)
    {
        std::unique_lock lock(m_mutex);
        if (m_size == m_items.size())
        {
            return false;
        }
        PushLocked(item);
        lock.unlock();
        m_not_empty.notify_one();
        return true;
    }

    bool Pop(T& result)
    {
        std::unique_lock lock(m_mutex);
        m_not_empty.wait(lock, [this] { return m_size != 0 || m_is_closed; });
        if (m_size == 0)
        {
            return false;
        }
        PopLocked(result);
        lock.unlock();
        m_not_full.notify_one();
        return true;
    }

    bool TryPop(T& result)
    {
        std::unique_lock lock(m_mutex);
        if (m_size == 0)
        {
            return false;
        }
        PopLocked(result);
        lock.unlock();
        m_not_full.notify_one();
        return true;
    }

    void Close()
    {
        {
            std::lock_guard lock(m_mutex);
            m_is_closed = true;
        }
        m_not_empty.notify_all();
    }

private:
    void PushLocked(const T& item)
    {
        m_items[(m_head + m_size) % m_items.size()] = item;
        m_size++;
    }

    void PopLocked(T& result)
    {
        result = m_items[m_head];
        m_head = (m_head + 1) % m_items.size();
        m_size--;
    }

    std::mutex m_mutex;
    std::condition_variable m_not_full;
    std::condition_variable m_not_empty;
    std::vector<T> m_items;
    size_t m_head = 0;
    size_t m_size = 0;
    bool m_is_closed = false;
};

using Std = MutexQueue<Item>;

/**
 * Uniform interface over the channel types so every benchmark body is written once. The channel stays with the benchmark thread,
 * and each worker thread is handed an end made by MakeTransmitter or MakeReceiver. An SPSC channel has only one of each.
 */
template <typename Channel>
struct ChannelOps;

template <bool UseSignaling>
struct ChannelOps<Opal::ChannelSPSC<Item, UseSignaling>>
{
    using Channel = Opal::ChannelSPSC<Item, UseSignaling>;
    using Transmitter = Opal::TransmitterSPSC<Item, UseSignaling>;
    using Receiver = Opal::ReceiverSPSC<Item, UseSignaling>;

    static std::unique_ptr<Channel> Make(size_t capacity) { return std::make_unique<Channel>(capacity); }
    static Transmitter MakeTransmitter(Channel& channel) { return Opal::Move(channel.transmitter); }
    static Receiver MakeReceiver(Channel& channel) { return Opal::Move(channel.receiver); }
    static void Send(Transmitter& tx, Item item) { tx.Send(item); }
    static bool TrySend(Transmitter& tx, Item item) { return tx.TrySend(item); }
    static void Close(Transmitter& tx) { tx.Close(); }

    static bool Receive(Receiver& rx, Item& item)
    {
        auto result = rx.Receive();
        if (!result.HasValue())
        {
            return false;
        }
        item = result.GetValue();
        return true;
    }

    static bool TryReceive(Receiver& rx, Item& item) { return rx.TryReceive(item) == Opal::ErrorCode::Success; }
};

template <bool UseSignaling>
struct ChannelOps<Opal::ChannelMPMC<Item, UseSignaling>>
{
    using Channel = Opal::ChannelMPMC<Item, UseSignaling>;
    using Transmitter = Opal::TransmitterMPMC<Item, UseSignaling>;
    using Receiver = Opal::ReceiverMPMC<Item, UseSignaling>;

    static std::unique_ptr<Channel> Make(size_t capacity) { return std::make_unique<Channel>(capacity); }
    static Transmitter MakeTransmitter(Channel& channel) { return channel.transmitter.Clone(); }
    static Receiver MakeReceiver(Channel& channel) { return channel.receiver.Clone(); }
    static void Send(Transmitter& tx, Item item) { tx.Send(item); }
    static bool TrySend(Transmitter& tx, Item item) { return tx.TrySend(item); }
    static void Close(Transmitter& tx) { tx.Close(); }

    static bool Receive(Receiver& rx, Item& item)
    {
        auto result = rx.Receive();
        if (!result.HasValue())
        {
            return false;
        }
        item = result.GetValue();
        return true;
    }

    static bool TryReceive(Receiver& rx, Item& item) { return rx.TryReceive(item) == Opal::ErrorCode::Success; }
};

template <bool UseSignaling>
struct ChannelOps<Opal::ChannelMPSC<Item, UseSignaling>>
{
    using Channel = Opal::ChannelMPSC<Item, UseSignaling>;
    using Transmitter = Opal::TransmitterMPSC<Item, UseSignaling>;
    using Receiver = Opal::ReceiverMPSC<Item, UseSignaling>;

    static std::unique_ptr<Channel> Make(size_t capacity) { return std::make_unique<Channel>(capacity); }
    static Transmitter MakeTransmitter(Channel& channel) { return channel.transmitter.Clone(); }
    static Receiver MakeReceiver(Channel& channel) { return Opal::Move(channel.receiver); }
    static void Send(Transmitter& tx, Item item) { tx.Send(item); }
    static bool TrySend(Transmitter& tx, Item item) { return tx.TrySend(item); }
    static void Close(Transmitter& tx) { tx.Close(); }

    static bool Receive(Receiver& rx, Item& item)
    {
        auto result = rx.Receive();
        if (!result.HasValue())
        {
            return false;
        }
        item = result.GetValue();
        return true;
    }

    static bool TryReceive(Receiver& rx, Item& item) { return rx.TryReceive(item) == Opal::ErrorCode::Success; }
};

template <>
struct ChannelOps<Std>
{
    using Channel = Std;
    using Transmitter = Std*;
    using Receiver = Std*;

    static std::unique_ptr<Channel> Make(size_t capacity) { return std::make_unique<Channel>(capacity); }
    static Transmitter MakeTransmitter(Channel& channel) { return &channel; }
    static Receiver MakeReceiver(Channel& channel) { return &channel; }
    static void Send(Transmitter& tx, Item item) { tx->Push(item); }
    static bool TrySend(Transmitter& tx, Item item) { return tx->TryPush(item); }
    static void Close(Transmitter& tx) { tx->Close(); }
    static bool Receive(Receiver& rx, Item& item) { return rx->Pop(item); }
    static bool TryReceive(Receiver& rx, Item& item) { return rx->TryPop(item); }
};

// Fill the channel to capacity from one thread, then empty it. Nothing waits, so this is the bare cost of a send and a receive.
template <typename Channel>
void BM_FillDrain(benchmark::State& state)
{
    using Ops = ChannelOps<Channel>;
    const size_t capacity = static_cast<size_t>(state.range(0));
    std::unique_ptr<Channel> channel = Ops::Make(capacity);
    typename Ops::Transmitter tx = Ops::MakeTransmitter(*channel);
    typename Ops::Receiver rx = Ops::MakeReceiver(*channel);

    for (auto _ : state)
    {
        size_t sent = 0;
        for (size_t i = 0; i < capacity; i++)
        {
            sent += Ops::TrySend(tx, i) ? 1u : 0u;
        }
        size_t received = 0;
        Item sum = 0;
        Item item = 0;
        for (size_t i = 0; i < capacity; i++)
        {
            received += Ops::TryReceive(rx, item) ? 1u : 0u;
            sum += item;
        }
        benchmark::DoNotOptimize(sum);
        if (sent != capacity || received != capacity)
        {
            state.SkipWithError("channel did not hold its full capacity");
            break;
        }
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(capacity));
}

// Hand k_transfer_items from the producer threads to the consumer threads. The last producer to finish closes the channel, and the
// consumers receive until it reports closed. Each thread is pinned to a core of its own as placement says. Thread startup is outside
// the timed region: the clock starts once every thread is pinned and waiting at the starting line and stops when the last one has
// exited.
template <typename Channel, Placement ThreadPlacement>
void BM_Transfer(benchmark::State& state)
{
    using Ops = ChannelOps<Channel>;
    const size_t capacity = static_cast<size_t>(state.range(0));
    const size_t producer_count = static_cast<size_t>(state.range(1));
    const size_t consumer_count = static_cast<size_t>(state.range(2));
    const size_t thread_count = producer_count + consumer_count;
    const std::vector<Opal::u32> cores = AssignCores(ThreadPlacement, producer_count, consumer_count);
    if (cores.empty())
    {
        state.SkipWithMessage("this machine does not have the physical cores or L3 caches for this placement");
        return;
    }

    const size_t items_per_producer = k_transfer_items / producer_count;
    const size_t total_items = items_per_producer * producer_count;
    const Item expected_sum = total_items * (total_items - 1) / 2;

    for (auto _ : state)
    {
        std::unique_ptr<Channel> channel = Ops::Make(capacity);
        std::vector<typename Ops::Transmitter> transmitters;
        std::vector<typename Ops::Receiver> receivers;
        for (size_t i = 0; i < producer_count; i++)
        {
            transmitters.push_back(Ops::MakeTransmitter(*channel));
        }
        for (size_t i = 0; i < consumer_count; i++)
        {
            receivers.push_back(Ops::MakeReceiver(*channel));
        }

        std::atomic<size_t> ready_count{0};
        std::atomic<bool> go{false};
        std::atomic<size_t> producers_left{producer_count};
        std::vector<Item> sums(consumer_count, 0);
        std::vector<size_t> counts(consumer_count, 0);
        auto wait_for_start = [&](size_t thread_index)
        {
            PinCurrentThread(cores[thread_index]);
            ready_count.fetch_add(1, std::memory_order_release);
            while (!go.load(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }
        };

        std::vector<std::thread> threads;
        threads.reserve(thread_count);
        for (size_t p = 0; p < producer_count; p++)
        {
            threads.emplace_back(
                [&, p]
                {
                    wait_for_start(p);
                    typename Ops::Transmitter& tx = transmitters[p];
                    const Item first = p * items_per_producer;
                    for (Item i = first; i < first + items_per_producer; i++)
                    {
                        Ops::Send(tx, i);
                    }
                    if (producers_left.fetch_sub(1, std::memory_order_acq_rel) == 1)
                    {
                        Ops::Close(tx);
                    }
                });
        }
        for (size_t c = 0; c < consumer_count; c++)
        {
            threads.emplace_back(
                [&, c]
                {
                    wait_for_start(producer_count + c);
                    typename Ops::Receiver& rx = receivers[c];
                    Item item = 0;
                    Item sum = 0;
                    size_t count = 0;
                    while (Ops::Receive(rx, item))
                    {
                        sum += item;
                        count++;
                    }
                    sums[c] = sum;
                    counts[c] = count;
                });
        }

        while (ready_count.load(std::memory_order_acquire) != thread_count)
        {
            std::this_thread::yield();
        }
        const auto start = std::chrono::steady_clock::now();
        go.store(true, std::memory_order_release);
        for (std::thread& thread : threads)
        {
            thread.join();
        }
        const auto end = std::chrono::steady_clock::now();
        state.SetIterationTime(std::chrono::duration<double>(end - start).count());

        Item sum = 0;
        size_t count = 0;
        for (size_t c = 0; c < consumer_count; c++)
        {
            sum += sums[c];
            count += counts[c];
        }
        if (count != total_items || sum != expected_sum)
        {
            state.SkipWithError("items were lost or duplicated in transfer");
            break;
        }
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(total_items));
}

// 64 slots sit in L1 for either channel. 16K MPMC slots take a cache line each, 1 MB, which is past a Zen 3 core's 512 KB L2.
constexpr int64_t k_capacities[] = {64, 1024, 16384};

void FillDrainSweep(benchmark::Benchmark* b)
{
    b->ArgName("capacity");
    for (const int64_t capacity : k_capacities)
    {
        b->Arg(capacity);
    }
}

void TransferSweepSPSC(benchmark::Benchmark* b)
{
    b->ArgNames({"capacity", "producers", "consumers"});
    for (const int64_t capacity : k_capacities)
    {
        b->Args({capacity, 1, 1});
    }
    b->UseManualTime();
}

// Producers and consumers: balanced, fan-in and fan-out.
constexpr std::pair<int64_t, int64_t> k_thread_shapes[] = {{1, 1}, {2, 2}, {4, 4}, {4, 1}, {7, 1}, {1, 4}};

// Producers into the one consumer of an MPSC channel. Seven fills a CCD of eight cores with the consumer.
constexpr int64_t k_fan_in_producers[] = {1, 2, 4, 7};

void TransferSweepMPSC(benchmark::Benchmark* b)
{
    b->ArgNames({"capacity", "producers", "consumers"});
    for (const int64_t capacity : k_capacities)
    {
        for (const int64_t producers : k_fan_in_producers)
        {
            b->Args({capacity, producers, 1});
        }
    }
    b->UseManualTime();
}

void TransferSweepMPMC(benchmark::Benchmark* b)
{
    b->ArgNames({"capacity", "producers", "consumers"});
    for (const int64_t capacity : k_capacities)
    {
        for (const auto& [producers, consumers] : k_thread_shapes)
        {
            b->Args({capacity, producers, consumers});
        }
    }
    b->UseManualTime();
}

}  // namespace

// Names come out as "<Channel>/<operation>[/<placement>]/<Signal|Spin|Std>/<args>". Std is the mutex queue, so the compare.py that
// ships with Google Benchmark pairs either Opal variant with it:
//   compare.py filters results.json "/Spin/" "/Std/"
#define CHANNEL_BENCHMARK(fn, name, signal, spin, configure)                     \
    BENCHMARK_TEMPLATE(fn, signal)->Name(name "/Signal")->Apply(configure); \
    BENCHMARK_TEMPLATE(fn, spin)->Name(name "/Spin")->Apply(configure);     \
    BENCHMARK_TEMPLATE(fn, Std)->Name(name "/Std")->Apply(configure)

#define TRANSFER_BENCHMARK(name, signal, spin, placement, configure)                                                        \
    BENCHMARK_TEMPLATE(BM_Transfer, signal, Placement::placement)->Name(name "/" #placement "/Signal")->Apply(configure); \
    BENCHMARK_TEMPLATE(BM_Transfer, spin, Placement::placement)->Name(name "/" #placement "/Spin")->Apply(configure);     \
    BENCHMARK_TEMPLATE(BM_Transfer, Std, Placement::placement)->Name(name "/" #placement "/Std")->Apply(configure)

CHANNEL_BENCHMARK(BM_FillDrain, "ChannelSPSC/FillDrain", SpscSignal, SpscSpin, FillDrainSweep);
TRANSFER_BENCHMARK("ChannelSPSC/Transfer", SpscSignal, SpscSpin, SameL3, TransferSweepSPSC);
TRANSFER_BENCHMARK("ChannelSPSC/Transfer", SpscSignal, SpscSpin, CrossL3, TransferSweepSPSC);
CHANNEL_BENCHMARK(BM_FillDrain, "ChannelMPMC/FillDrain", MpmcSignal, MpmcSpin, FillDrainSweep);
TRANSFER_BENCHMARK("ChannelMPMC/Transfer", MpmcSignal, MpmcSpin, SameL3, TransferSweepMPMC);
TRANSFER_BENCHMARK("ChannelMPMC/Transfer", MpmcSignal, MpmcSpin, CrossL3, TransferSweepMPMC);
CHANNEL_BENCHMARK(BM_FillDrain, "ChannelMPSC/FillDrain", MpscSignal, MpscSpin, FillDrainSweep);
TRANSFER_BENCHMARK("ChannelMPSC/Transfer", MpscSignal, MpscSpin, SameL3, TransferSweepMPSC);
TRANSFER_BENCHMARK("ChannelMPSC/Transfer", MpscSignal, MpscSpin, CrossL3, TransferSweepMPSC);
