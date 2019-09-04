#include "gtest/gtest.h"

#include <toucan/core/scheduler.hpp>
#include <toucan/algo/fifo.hpp>
#include <toucan/core/api.hpp>

#include <iostream>
#include <chrono>

using namespace toucan;
using namespace algo;

TEST(FIFO, DefaultConstructor) {
    Scheduler scheduler(std::make_shared<FIFO>());
}

TEST(FIFO, Constructor) {
    Scheduler scheduler(std::make_shared<FIFO>(), 5);
}

TEST(FIFO, SimpleSpawn) {
    for (size_t i = 0; i != 10000; ++i) {
        Scheduler scheduler(std::make_shared<FIFO>());
        scheduler.Spawn([&] {
        });
        scheduler.WaitAll();
    }
}

TEST(FIFO, Spawn) {
    Scheduler scheduler(std::make_shared<FIFO>());
    std::atomic<bool> flag = false;
    scheduler.Spawn([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        flag = true;
    });
    ASSERT_FALSE(flag);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ASSERT_TRUE(flag);
}

TEST(FIFO, Wait) {
    Scheduler scheduler(std::make_shared<FIFO>());
    std::atomic<bool> flag = false;
    scheduler.Spawn([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        flag = true;
    });
    ASSERT_FALSE(flag);
    scheduler.WaitAll();
    ASSERT_TRUE(flag);
}
        

TEST(FIFO, Yield) {
    Scheduler scheduler(std::make_shared<FIFO>());
    const size_t kIters = 100;
    const size_t kTasks = 100;
    std::atomic<size_t> count = 0;
    auto task = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            Yield();
        }
        ++count;
    };
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    scheduler.WaitAll();
    ASSERT_EQ(count, kTasks);
}

TEST(FIFO, Terminate) {
    Scheduler scheduler(std::make_shared<FIFO>());
    std::atomic<int> count = 0;
    const size_t kTasks = 100;
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn([&count, i] {
            if (i % 2 == 0) {
                Terminate();
            }
            ++count;
        });
    }
    scheduler.WaitAll();
    ASSERT_EQ(2 * count, kTasks);
}

TEST(FIFO, APISPawn) {
    Scheduler scheduler(std::make_shared<FIFO>());
    std::atomic<bool> flag = false;
    scheduler.Spawn([&] {
        Spawn([&] {
            flag = true;
        });
    });
    scheduler.WaitAll();
    ASSERT_TRUE(flag);
}

void recursive_function(size_t layer, std::atomic<size_t>& count) {
    Spawn([layer, &count] {
        if (layer > 0) {
            recursive_function(layer - 1, count);
        }
        ++count;
    });
}

TEST(FIFO, Recursive) {
    Scheduler scheduler(std::make_shared<FIFO>());
    std::atomic<size_t> count{0};
    const size_t kLayers = 1000;
    scheduler.Spawn([kLayers, &count] {
        recursive_function(kLayers, count);
    });
    scheduler.WaitAll();
    ASSERT_EQ(count.load(), kLayers + 1);
}

void recursive_binary_function(size_t layer, std::atomic<size_t>& count) {
    Spawn([layer, &count] {
        if (layer > 0) {
            recursive_binary_function(layer - 1, count);
        }
        ++count;
    });
    Spawn([layer, &count] {
        if (layer > 0) {
            recursive_binary_function(layer - 1, count);
        }
        ++count;
    });
}

TEST(FIFO, RecursiveBinaryTree) {
    Scheduler scheduler(std::make_shared<FIFO>());
    std::atomic<size_t> count{0};
    const size_t kLayers = 10;
    scheduler.Spawn([kLayers, &count] {
        recursive_binary_function(kLayers, count);
        ++count;
    });
    scheduler.WaitAll();
    ASSERT_EQ(count.load(), (1u<<(kLayers + 2)) - 1);
}

TEST(FIFO, Parallel) {
    Scheduler scheduler(std::make_shared<FIFO>(), 4);
    std::atomic<size_t> count = 0;
    const size_t kTasks = 8;
    const size_t kCycles = 10;
    const auto kIdleTime = std::chrono::milliseconds(10);
    auto task = [&] {
        for (size_t i = 0; i != kCycles; ++i) {
            std::this_thread::sleep_for(kIdleTime);
            Yield();
        }
        ++count;
    };
    auto start = std::chrono::system_clock::now();
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    ASSERT_NE(count, kTasks);
    scheduler.WaitAll();
    auto end = std::chrono::system_clock::now();
    ASSERT_LT(2 * (end - start), kTasks * kCycles * kIdleTime);
    ASSERT_EQ(count, kTasks);
}

