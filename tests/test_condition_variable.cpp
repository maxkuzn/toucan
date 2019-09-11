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

/////////////////////////////////////////////////////////////////////

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

