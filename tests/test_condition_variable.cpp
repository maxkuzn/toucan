#include "gtest/gtest.h"

#include <toucan/core/scheduler.hpp>
#include <toucan/core/api.hpp>
#include <toucan/sync/mutex.hpp>
#include <toucan/sync/unique_lock.hpp>
#include <toucan/sync/condition_variable.hpp>

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

TEST_CASE_WITH_ALL_ALGOS(ConditionVariableTest);

TYPED_TEST(ConditionVariableTest, Works) {
    Mutex mutex;
    ConditionVariable cv;

    static const size_t kPawns = 20;
    static const size_t kIters = 100;
    FiberBarrier barrier(kPawns + 1);
    size_t fuel = 0;

    auto boss = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            barrier.PassThrough();
            for (size_t j = 0; j != kPawns; ++j) {
                mutex.Lock();
                fuel++;
                auto old_fuel = fuel;
                cv.NotifyOne();
                Yield();
                ASSERT_EQ(fuel, old_fuel);
                mutex.Unlock();
                Yield();
            }

            barrier.PassThrough();
            ASSERT_EQ(fuel, 0);

            mutex.Lock();
            fuel += kPawns;
            cv.NotifyAll();
            Yield();
            ASSERT_EQ(fuel, kPawns);
            mutex.Unlock();
        }
    };

    auto pawn = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            barrier.PassThrough();
            UniqueLock lock(mutex);
            while (fuel == 0) {
                cv.Wait(lock);
            }
            fuel--;

            lock.Unlock();
            barrier.PassThrough();
            lock.Lock();
            cv.Wait(lock, [&] { return fuel != 0; });
            fuel--;
        }
    };

    auto scheduler = Scheduler::Create<TypeParam>(4);
    for (size_t i = 0; i != kPawns; ++i) {
        scheduler.Spawn(pawn);
    }
    scheduler.Spawn(boss);
    scheduler.WaitAll();
    
    ASSERT_EQ(fuel, 0);
}

TYPED_TEST(ConditionVariableTest, NoBusyWait) {
    Mutex mutex;
    ConditionVariable cv;
    bool flag = true;

    auto holder = [&] {
        UniqueLock lock(mutex);
        while (flag) {
            lock.Unlock();
            Yield();
            lock.Lock();
        }
        std::this_thread::sleep_for(1s);
        flag = true;
        cv.NotifyOne();
    };

    auto waiter = [&] {
        UniqueLock lock(mutex);
        flag = false;
        auto start = std::chrono::steady_clock::now();
        cv.Wait(lock, [&] { return flag; });
        auto end = std::chrono::steady_clock::now();
        ASSERT_GT(end - start, 900ms);
    };

    auto scheduler = Scheduler::Create<TypeParam>(4);

    auto start_clock = std::clock();
    scheduler.Spawn(holder);
    scheduler.Spawn(waiter);
    scheduler.WaitAll();

    auto cpu_time_seconds = static_cast<double>(std::clock() - start_clock) / CLOCKS_PER_SEC;
    ASSERT_LT(cpu_time_seconds, 0.1);
}

std::vector<int> GenerateRandomVector(size_t size) {
    static std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    std::vector<int> v(size);
    for (auto& n : v) {
        n = dis(gen);
    }
    return v;
}

void Merge(std::vector<int>& data, std::vector<int>& buff,
           size_t from, size_t middle, size_t to) {
    size_t l = from;
    size_t r = middle;
    size_t buff_idx = from;
    while (l != middle && r != to) {
        if (data[l] < data[r]) {
            buff[buff_idx++] = data[l++];
        } else {
            buff[buff_idx++] = data[r++];
        }
    }
    while (l != middle) {
        buff[buff_idx++] = data[l++];
    }
    while (r != to) {
        buff[buff_idx++] = data[r++];
    }
    for (size_t i = from; i != to; ++i) {
        data[i] = buff[i];
    }
}

void Task(size_t from, size_t to,
        std::vector<int>& data, std::vector<int>& buff,
        size_t& ready, Mutex& mutex, ConditionVariable& cv) {
    if (to - from >= 2) {
        size_t my_ready = 0;
        Mutex my_mutex;
        ConditionVariable my_cv;
        size_t middle = from + (to - from) / 2;
        Spawn([&] {
            Task(from, middle, data, buff, my_ready, my_mutex, my_cv);
        });
        Spawn([&] {
            Task(middle, to, data, buff, my_ready, my_mutex, my_cv);
        });
        UniqueLock my_lock(my_mutex);
        my_cv.Wait(my_lock, [&] { return my_ready == 2; });
        Merge(data, buff, from, middle, to);
    }
    UniqueLock lock(mutex);
    ready++;
    cv.NotifyAll();
}

TYPED_TEST(ConditionVariableTest, MergeSort) {
    const size_t kSize = 1024 * 128;

    std::vector<int> data = GenerateRandomVector(kSize);
    std::vector<int> buff(kSize);
    auto scheduler = Scheduler::Create<TypeParam>(4);
    scheduler.Spawn([&] {
        size_t ready = 0;
        Mutex mutex;
        ConditionVariable cv;
        Spawn([&] {
            Task(0, kSize, data, buff, ready, mutex, cv);
        });
        UniqueLock lock(mutex);
        cv.Wait(lock, [&] { return ready == 1; });
    });
    scheduler.WaitAll();
    for (size_t i = 0; i + 1 != kSize; ++i) {
        ASSERT_LE(data[i], data[i + 1]);
    }
}


/////////////////////////////////////////////////////////////////////

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

