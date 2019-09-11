#include "gtest/gtest.h"

#include <toucan/core/scheduler.hpp>
#include <toucan/core/api.hpp>
#include <toucan/sync/mutex.hpp>
#include <toucan/sync/unique_lock.hpp>

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

TEST_CASE_WITH_ALL_ALGOS(UniqueLockTest);

TYPED_TEST(UniqueLockTest, SimpleCounting) {
    static const size_t kTasks = 10;
    static const size_t kIncrements = 100;

    size_t count = 0;
    FiberBarrier barrier(kTasks);
    Mutex mutex;

    auto task = [&] {
        barrier.PassThrough();
        for (size_t i = 0; i != kIncrements; ++i) {
            UniqueLock lock(mutex);
            ++count;
        }
    };

    auto scheduler = Scheduler::Create<TypeParam>(4);
    for (size_t i = 0; i != kTasks; ++i) {
        scheduler.Spawn(task);
    }
    scheduler.WaitAll();

    ASSERT_EQ(count, kTasks * kIncrements);
}

/////////////////////////////////////////////////////////////////////

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

