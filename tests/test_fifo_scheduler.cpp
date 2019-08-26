#include "gtest/gtest.h"

#include <toucan/algo/fifo.hpp>

#include <iostream>

using namespace toucan;
using namespace algo;

TEST(FIFO, DefaultConstructor) {
    FIFOScheduler scheduler;
}

TEST(FIFO, Constructor) {
    FIFOScheduler scheduler(5);
}

TEST(FIFO, Spawn) {
    FIFOScheduler scheduler;
    bool flag = false;
    scheduler.Spawn([&] { flag = true; });
    ASSERT_TRUE(flag);
}

