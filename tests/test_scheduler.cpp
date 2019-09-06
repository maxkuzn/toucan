#include "gtest/gtest.h"

#include <toucan/core/scheduler.hpp>
#include <toucan/core/api.hpp>

#include "support/fiber_barrier.hpp"
#include "support/thread_counter.hpp"
#include "support/all_algos.hpp"

#include <cmath>
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

TEST_CASE_WITH_ALL_ALGOS(SchedulerTest);


TYPED_TEST(SchedulerTest, Destructor) {
    Scheduler scheduler(this->algo, 4);
    scheduler.Spawn([] {});
    scheduler.WaitAll();
}

TYPED_TEST(SchedulerTest, HardwareConcurrency) {
    Scheduler scheduler(this->algo);

    const size_t hw_concurrency = std::thread::hardware_concurrency();
    if (hw_concurrency != 0) {
        ASSERT_EQ(scheduler.ThreadCount(), hw_concurrency);
    } else {
        ASSERT_GT(scheduler.ThreadCount(), 0);
    }
}


TYPED_TEST(SchedulerTest, RunOneFiber) {
    Scheduler scheduler(this->algo, 4);
    std::atomic<bool> flag = false;
    scheduler.Spawn([&] {
        std::this_thread::sleep_for(5ms);
        flag = true;
    });
    ASSERT_FALSE(flag);
    scheduler.WaitAll();
    ASSERT_TRUE(flag);
}

TYPED_TEST(SchedulerTest, MoreFibers) {
    static const size_t kTasks = 100;
    std::atomic<size_t> count = 0;
    auto task = [&] {
        std::this_thread::sleep_for(2ms);
        count++;
    };

    Scheduler scheduler(this->algo, 4);
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    ASSERT_NE(count, kTasks);

    scheduler.WaitAll();
    ASSERT_EQ(count, kTasks);
}

// Doesn't work now because of workers busy wait for fibers
TYPED_TEST(SchedulerTest, DISABLED_NoBusyWorkers) {
    Scheduler scheduler(this->algo);
    auto start = std::clock();
    std::this_thread::sleep_for(500ms);
    auto cpu_elapsed_seconds = static_cast<double>(std::clock() - start) / CLOCKS_PER_SEC;
    ASSERT_LT(cpu_elapsed_seconds, 0.05);
}

TYPED_TEST(SchedulerTest, Barriers) {
    const size_t kTasks = 100;
    const size_t kIters = 1000;

    FiberBarrier barrier(kTasks);
    std::atomic<size_t> count = 0;

    auto task = [&]() {
        for (size_t i = 0; i != kIters; ++i) {
            ASSERT_LT(count, (i + 1) * kTasks);
            ++count;
            barrier.PassThrough();
            ASSERT_GE(count, (i + 1) * kTasks);
        }
    };

    Scheduler scheduler(this->algo, 4);
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    scheduler.WaitAll();
}

TYPED_TEST(SchedulerTest, YieldAndFairness) {
    static const size_t kIters = 1000;
    FiberBarrier barrier(2);
    std::atomic<int> first_count = 0;
    std::atomic<int> second_count = 0;

    auto first = [&] {
        barrier.PassThrough();
        for (size_t i = 0; i != kIters; ++i) {
            ++first_count;
            Yield();
            ASSERT_LE(std::abs(first_count - second_count), 10);
        }
    };

    auto second = [&] {
        barrier.PassThrough();
        for (size_t i = 0; i != kIters; ++i) {
            ++second_count;
            Yield();
            ASSERT_LE(std::abs(first_count - second_count), 10);
        }
    };

    Scheduler scheduler(this->algo, 1);
    scheduler.Spawn(first);
    scheduler.Spawn(second);
    scheduler.WaitAll();

    ASSERT_EQ(first_count, kIters);
    ASSERT_EQ(second_count, kIters);
}

TYPED_TEST(SchedulerTest, ThreadCount) {
    static const size_t kThreads = 13;
    static const size_t kTasks = 100;
    static const size_t kIters = 100;

    ThreadCounter thread_counter;

    auto task = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            thread_counter.Touch();
            Yield();
        }
    };

    auto spawner = [&] {
        for (size_t i = 0; i != kTasks; ++i) {
            thread_counter.Touch();
            Spawn(task);
            Yield();
        }
    };

    Scheduler scheduler(this->algo, kThreads);
    scheduler.Spawn(spawner);
    scheduler.WaitAll();

    ASSERT_EQ(thread_counter.ThreadCount(), kThreads);
}

TYPED_TEST(SchedulerTest, Races) {
    static const size_t kIncrements = 10000;
    static const size_t kFibers = 100;

    FiberBarrier start_barrier(kFibers);

    std::atomic<size_t> counter = 0;

    auto incrementer = [&]() {
        start_barrier.PassThrough();

        for (size_t i = 0; i != kIncrements; ++i) {
            counter.store(counter.load() + 1);
            Yield();
        }
    };

    Scheduler scheduler(this->algo, 4);
    for (size_t i = 0; i != kFibers; ++i) {
        scheduler.Spawn(incrementer);
    }
    scheduler.WaitAll();

    ASSERT_NE(counter, kIncrements * kFibers);
}

TYPED_TEST(SchedulerTest, NoRacesInSingleThread) {
    static const size_t kIncrements = 10000;
    static const size_t kFibers = 100;

    FiberBarrier start_barrier(kFibers);

    size_t counter = 0;

    auto incrementer = [&]() {
        start_barrier.PassThrough();

        for (size_t i = 0; i != kIncrements; ++i) {
            ++counter;
            Yield();
        }
    };

    Scheduler scheduler(this->algo, 1);
    for (size_t i = 0; i != kFibers; ++i) {
        scheduler.Spawn(incrementer);
    }
    scheduler.WaitAll();

    ASSERT_EQ(counter, kIncrements * kFibers);
}

TYPED_TEST(SchedulerTest, SpawnNotOnlyInCurrentThread) {
    static const size_t kIncrements = 10000;
    static const size_t kTasks = 100;

    std::atomic<size_t> counter = 0;
    std::atomic<size_t> completed_tasks = 0;

    auto task = [&]() {
        for (size_t i = 0; i != kIncrements; ++i) {
            counter.store(counter.load() + 1);
            Yield();
        }
        ++completed_tasks;
    };

    auto spawner = [&]() {
        for (size_t i = 0; i != kTasks; ++i) {
            Spawn(task);
            Yield();
        }
    };

    Scheduler scheduler(this->algo, 4);

    scheduler.Spawn(spawner);
    scheduler.WaitAll();

    ASSERT_EQ(completed_tasks, kTasks);
    ASSERT_NE(counter, kIncrements * kTasks);
}

/////////////////////////////////////////////////////////////////////

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

