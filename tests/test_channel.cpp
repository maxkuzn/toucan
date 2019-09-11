#include "gtest/gtest.h"

#include <toucan/core/scheduler.hpp>
#include <toucan/core/api.hpp>
#include <toucan/sync/channel.hpp>

#include "support/fiber_barrier.hpp"
#include "support/all_algos.hpp"

#include <ctime>
#include <chrono>

using namespace toucan;
using namespace toucan::testing;

// for 1s, 5ms and so on
using namespace std::chrono_literals;

/////////////////////////////////////////////////////////////////////

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

/////////////////////////////////////////////////////////////////////
// Tests

TEST_CASE_WITH_ALL_ALGOS(ChannelTest);

TYPED_TEST(ChannelTest, OnePair) {
    static const size_t kIncrements = 1000;

    size_t count = 0;
    FiberBarrier barrier(2);
    Channel<int> chan;

    auto alice = [&] {
        barrier.PassThrough();
        for (size_t i = 0; i != kIncrements; ++i) {
            chan.Put(1);
        }
    };

    auto bob = [&] {
        barrier.PassThrough();
        for (size_t i = 0; i != kIncrements; ++i) {
            int x = 0;
            ASSERT_TRUE(chan.Get(x));
            count += x;
        }
    };

    auto scheduler = Scheduler::Create<TypeParam>(4);
    scheduler.Spawn(alice);
    scheduler.Spawn(bob);
    scheduler.WaitAll();

    ASSERT_EQ(count, kIncrements);
}

TYPED_TEST(ChannelTest, OneSenderManyGetters) {
    static const size_t kWorkers = 100;
    static const size_t kNumbers = 100000;

    Channel<int> chan;

    std::mt19937 gen(42);
    static const int kMax = 1u << 10;
    std::uniform_int_distribution<> dis(-kMax, kMax);

    int sender_sum = 0;
    std::atomic<int> workers_sum{0};

    auto sender = [&] {
        for (size_t i = 0; i != kNumbers; ++i) {
            int x = dis(gen);
            sender_sum += x;
            chan.Put(x);
            Yield();
        }
        ASSERT_FALSE(chan.IsClosed());
        chan.Close();
        ASSERT_TRUE(chan.IsClosed());
    };

    auto worker = [&] {
        int x;
        int sum = 0;
        while (chan.Get(x)) {
            sum += x;
        }
        ASSERT_TRUE(chan.IsClosed());
        ASSERT_NE(sum, 0);
        workers_sum += sum;
    };

    auto scheduler = Scheduler::Create<TypeParam>(4);
    for (size_t i = 0; i != kWorkers; ++i) {
        scheduler.Spawn(worker);
    }
    scheduler.Spawn(sender);
    scheduler.WaitAll();

    ASSERT_EQ(workers_sum, sender_sum);
}

// Doesn't work now because of workers busy wait for fibers
TYPED_TEST(ChannelTest, NoBusyWait) {
    FiberBarrier barrier(2);
    Channel<int> chan;

    auto scheduler = Scheduler::Create<TypeParam>(4);
    scheduler.Spawn([&] {
        std::this_thread::sleep_for(1s);
        chan.Put(42);
    });

    std::this_thread::sleep_for(200ms);

    auto start_clock = std::clock();

    scheduler.Spawn([&] {
        auto start = std::chrono::steady_clock::now();
        int x;
        ASSERT_TRUE(chan.Get(x));
        ASSERT_EQ(x, 42);
        auto end = std::chrono::steady_clock::now();
        ASSERT_GT(end - start, 500ms);
    });

    scheduler.WaitAll();

    auto cpu_time_seconds = static_cast<double>(std::clock() - start_clock) / CLOCKS_PER_SEC;
    ASSERT_LT(cpu_time_seconds, 0.1);
}

TYPED_TEST(ChannelTest, DontBlockThread) {
    Channel<int> chan;

    auto waiter = [&chan]() {
        int x;
        ASSERT_FALSE(chan.Get(x));
    };

    static const size_t kWorkerSteps = 10000;

    std::atomic<size_t> worker_steps = 0;

    auto worker = [&chan, &worker_steps]() {
        for (size_t i = 0; i != kWorkerSteps; ++i) {
            worker_steps.store(worker_steps.load() + 1);
            Yield();
        }
        chan.Close();
    };

    auto scheduler = Scheduler::Create<TypeParam>(1);
    scheduler.Spawn(waiter);

    std::this_thread::sleep_for(200ms);

    scheduler.Spawn(worker);

    std::this_thread::sleep_for(500ms);
    ASSERT_EQ(worker_steps, kWorkerSteps);

    scheduler.WaitAll();
}

/////////////////////////////////////////////////////////////////////

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

