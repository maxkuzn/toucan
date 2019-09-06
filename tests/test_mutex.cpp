#include "gtest/gtest.h"

#include <toucan/core/scheduler.hpp>
#include <toucan/core/api.hpp>
#include <toucan/sync/mutex.hpp>

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

TEST_CASE_WITH_ALL_ALGOS(MutexTest);

TYPED_TEST(MutexTest, SimpleCounting) {
    static const size_t kTasks = 100;
    static const size_t kIncrements = 1000;

    size_t count = 0;
    FiberBarrier barrier(kTasks);
    Mutex mutex;

    auto task = [&] {
        barrier.PassThrough();
        for (size_t i = 0; i != kIncrements; ++i) {
            mutex.Lock();
            ++count;
            mutex.Unlock();
        }
    };

    Scheduler scheduler(this->algo, 4);
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    scheduler.WaitAll();

    ASSERT_EQ(count, kTasks * kIncrements);
}

// Doesn't work now because of workers busy wait for fibers
TYPED_TEST(MutexTest, DISABLED_NoBusyWait) {
    FiberBarrier barrier(2);
    Mutex mutex;

    Scheduler scheduler(this->algo, 4);
    scheduler.Spawn([&] {
        mutex.Lock();
        std::this_thread::sleep_for(1s);
        mutex.Unlock();
    });

    std::this_thread::sleep_for(200ms);

    auto start_clock = std::clock();

    scheduler.Spawn([&] {
        auto start = std::chrono::steady_clock::now();
        mutex.Lock();
        auto end = std::chrono::steady_clock::now();
        ASSERT_GT(end - start, 500ms);
        mutex.Unlock();
    });

    scheduler.WaitAll();

    auto cpu_time_seconds = static_cast<double>(std::clock() - start_clock) / CLOCKS_PER_SEC;
    ASSERT_LT(cpu_time_seconds, 0.1);
}

TYPED_TEST(MutexTest, DontBlockThread) {
    Mutex mutex;

    auto holder = [&mutex]() {
        mutex.Lock();

        auto start = std::chrono::steady_clock::now();
        auto end = start;
        do {
            Yield();
            end = std::chrono::steady_clock::now();
        } while (end - start < 1s);

        mutex.Unlock();
    };

    auto waiter = [&mutex]() {
        mutex.Lock();
        mutex.Unlock();
    };

    static const size_t kWorkerSteps = 10000;

    std::atomic<size_t> worker_steps = 0;

    auto worker = [&worker_steps]() {
        for (size_t i = 0; i != kWorkerSteps; ++i) {
            worker_steps.store(worker_steps.load() + 1);
            Yield();
        }
    };

    Scheduler scheduler(this->algo, 1);
    scheduler.Spawn(holder);

    std::this_thread::sleep_for(200ms);

    scheduler.Spawn(waiter);
    scheduler.Spawn(worker);

    std::this_thread::sleep_for(500ms);
    ASSERT_EQ(worker_steps, kWorkerSteps);

    scheduler.WaitAll();
}

/////////////////////////////////////////////////////////////////////

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

