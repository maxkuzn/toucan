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
    Scheduler scheduler(std::make_shared<FIFO>(), 2);
}

// not stable test
TEST(FIFO, Spawn) {
    Scheduler scheduler(std::make_shared<FIFO>());
    bool flag = false;
    scheduler.Spawn([&] { flag = true; });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_TRUE(flag);
}

TEST(FIFO, Yield) {
    Scheduler scheduler(std::make_shared<FIFO>());
    bool flag = false;
    scheduler.Spawn([&] {
        Yield();
        flag = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_TRUE(flag);
}

TEST(FIFO, Terminate) {
    Scheduler scheduler(std::make_shared<FIFO>());
    bool flag = false;
    scheduler.Spawn([&] {
        Terminate();
        flag = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_FALSE(flag);
}

TEST(FIFO, APISPawn) {
    Scheduler scheduler(std::make_shared<FIFO>());
    bool flag = false;
    scheduler.Spawn([&] {
        Spawn([&] {
            flag = true;
        });
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_TRUE(flag);
}

TEST(FIFO, MoreWork) {
    Scheduler scheduler(std::make_shared<FIFO>());
    std::atomic<size_t> count{0};
    static const size_t kIncrs = 10000;
    for (size_t i = 0; i != kIncrs; ++i) {
        scheduler.Spawn([&] {
            ++count;
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(count.load(), kIncrs);
}

