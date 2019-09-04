#include "gtest/gtest.h"

#include <toucan/core/scheduler.hpp>
#include <toucan/algo/fifo.hpp>
#include <toucan/core/api.hpp>

#include <toucan/sync/mutex.hpp>

#include <iostream>
#include <chrono>

using namespace toucan;
using namespace algo;

TEST(Mutex, LockUnlock) {
    Scheduler scheduler(std::make_shared<FIFO>(), 4);
    toucan::Mutex mutex;
    scheduler.Spawn([&] {
        mutex.Lock();
        mutex.Unlock();
    });
    scheduler.WaitAll();
}

TEST(Mutex, Explicit) {
    Scheduler scheduler(std::make_shared<FIFO>(), 4);
    static const size_t kTasks = 10;
    static const size_t kIters = 10;
    toucan::Mutex mutex;
    static const auto kIdleTime = std::chrono::milliseconds(1);
    std::atomic<size_t> count = 0;
    auto task = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            mutex.Lock();
            std::this_thread::sleep_for(kIdleTime);
            mutex.Unlock();
        }
        ++count;
    };
    auto start = std::chrono::system_clock::now();
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    scheduler.WaitAll();
    auto end = std::chrono::system_clock::now();
    ASSERT_EQ(count, kTasks);
    ASSERT_GT(end - start, kTasks * kIters * kIdleTime);
}

TEST(Mutex, Stress) {
    Scheduler scheduler(std::make_shared<FIFO>(), 8);
    std::atomic<size_t> count = 0;
    static const size_t kTasks = 100;
    static const size_t kCycles = 100;
    toucan::Mutex mutex;
    size_t sum = 0;
    auto task = [&] {
        for (size_t i = 0; i != kCycles; ++i) {
            mutex.Lock();
            Yield();
            ++sum;
            mutex.Unlock();
        }
        ++count;
    };
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    scheduler.WaitAll();
    ASSERT_EQ(count, kTasks);
    ASSERT_EQ(sum, kTasks * kCycles);
}

