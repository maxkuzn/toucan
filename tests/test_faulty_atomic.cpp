#include "gtest/gtest.h"

#include <toucan/fault/atomic.hpp>

using namespace toucan::fault;

TEST(FaultyAtomic, DefaultConstructor) {
    FaultyAtomic<int> x;
}

TEST(FaultyAtomic, Constructor) {
    FaultyAtomic<int> x(100);
    ASSERT_EQ(x.load(), 100);
}

TEST(FaultyAtomic, StoreAndLoad) {
    FaultyAtomic<bool> x{false};
    ASSERT_FALSE(x.load());
    x.store(true);
    ASSERT_TRUE(x.load());
    x.store(false);
    ASSERT_FALSE(x.load());
}

TEST(FaultyAtomic, Operators) {
    FaultyAtomic<bool> x(false);
    ASSERT_FALSE(x);
    x = true;
    ASSERT_TRUE(x);
    x = false;
    ASSERT_FALSE(x);
}

TEST(FaultyAtomic, Exchange) {
    FaultyAtomic<int> x(0);

    ASSERT_EQ(x.exchange(100), 0);
    ASSERT_EQ(x.load(), 100);

    ASSERT_EQ(x.exchange(-42), 100);
    ASSERT_EQ(x.load(), -42);
}

TEST(FaultyAtomic, FetchAddSub) {
    FaultyAtomic<int> x(0);

    ASSERT_EQ(x.fetch_add(1), 0);
    ASSERT_EQ(x.load(), 1);

    ASSERT_EQ(x.fetch_sub(1), 1);
    ASSERT_EQ(x.load(), 0);

    ASSERT_EQ(x.fetch_add(42), 0);
    ASSERT_EQ(x.load(), 42);

    ASSERT_EQ(x.fetch_sub(123), 42);
    ASSERT_EQ(x.load(), -81);
}

TEST(FaultyAtomic, AddSubOperators) {
    FaultyAtomic<int> x(0);

    x += 10;
    ASSERT_EQ(x.load(), 10);

    x -= 5;
    ASSERT_EQ(x.load(), 5);

    ASSERT_EQ(++x, 6);
    ASSERT_EQ(x.load(), 6);

    ASSERT_EQ(x++, 6);
    ASSERT_EQ(x.load(), 7);

    ASSERT_EQ(--x, 6);
    ASSERT_EQ(x.load(), 6);

    ASSERT_EQ(x--, 6);
    ASSERT_EQ(x.load(), 5);
}

TEST(FaultyAtomic, CompareExchange) {
    int expected;
    FaultyAtomic<int> x(0);

    x = 5;
    ASSERT_EQ(x.load(), 5);

    expected = 2;
    ASSERT_FALSE(x.compare_exchange_weak(expected, 20));
    ASSERT_EQ(x.load(), 5);
    ASSERT_EQ(expected, 5);

    expected = 5;
    ASSERT_TRUE(x.compare_exchange_strong(expected, 20));
    ASSERT_EQ(x.load(), 20);
}

TEST(FaultyAtomic, MemoryOrder) {
    int expected;
    FaultyAtomic<int> x(0);

    x.store(5, std::memory_order_release);
    ASSERT_EQ(5, x.load(std::memory_order_acquire));

    ASSERT_EQ(5, x.fetch_add(10, std::memory_order_relaxed));
    ASSERT_EQ(15, x.fetch_sub(5, std::memory_order_relaxed));

    ASSERT_EQ(10, x.exchange(42, std::memory_order_relaxed));

    expected = 42;
    ASSERT_TRUE(x.compare_exchange_weak(expected, 1, std::memory_order_relaxed, std::memory_order_seq_cst));

    expected = 1;
    ASSERT_TRUE(x.compare_exchange_weak(expected, 2, std::memory_order_seq_cst));

    expected = 2;
    ASSERT_TRUE(x.compare_exchange_weak(expected, 3, std::memory_order_seq_cst, std::memory_order_relaxed));

    expected = 3;
    ASSERT_TRUE(x.compare_exchange_weak(expected, 4, std::memory_order_relaxed));
}

