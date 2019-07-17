#include <gtest/gtest.h>
#include <toucan/scheduler.hpp>

#include <atomic>
#include <mutex>
#include <thread>
#include <set>

using namespace toucan;

TEST(SchedulerConstructor, Default) {
    Scheduler schedule;
}

TEST(SchedulerConstructor, SomeNumberOfThreads) {
    Scheduler schedule(8);
}

TEST(SchedulerConstructor, ZeroThreads) {
    Scheduler schedule(0);
}

TEST(Scheduler, Works) {
    Scheduler scheduler;
    std::atomic<size_t> counter{0};
    const size_t N = 10000;
    for (size_t i = 0; i != N; ++i) {
        scheduler.Submit([&] {
            ++counter;
        });
    }
    ASSERT_NE(counter.load(), 0);
    scheduler.WaitAll();
    ASSERT_EQ(counter.load(), N);
}

TEST(Scheduler, WorksParallel) {
    std::set<std::thread::id> ids;
    std::mutex mtx;
    const size_t N = 10000;
    const size_t threads = 8;
    Scheduler scheduler(threads);
    for (size_t i = 0; i != N; ++i) {
        scheduler.Submit([&] {
            mtx.lock();
            ids.insert(std::this_thread::get_id());
            mtx.unlock();
        });
    }
    scheduler.WaitAll();
    ASSERT_EQ(ids.size(), threads);
}

TEST(Scheduler, WaitAll) {
    Scheduler scheduler(2);
    std::atomic<size_t> counter{0};
    const size_t N = 10;
    for (size_t i = 0; i != N; ++i) {
        scheduler.Submit([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ++counter;
        });
    }
    ASSERT_NE(counter.load(), N);
    scheduler.WaitAll();
    ASSERT_EQ(counter.load(), N);
}


TEST(Scheduler, DefaultConstructorUseHardwareConcurrency) {
    std::set<std::thread::id> ids;
    std::mutex mtx;
    const size_t N = 10000;
    Scheduler scheduler;
    for (size_t i = 0; i != N; ++i) {
        scheduler.Submit([&] {
            mtx.lock();
            ids.insert(std::this_thread::get_id());
            mtx.unlock();
        });
    }
    scheduler.WaitAll();
    ASSERT_EQ(ids.size(), std::thread::hardware_concurrency());
}

