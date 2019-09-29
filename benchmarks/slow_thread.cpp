#include <benchmark/benchmark.h>

#include <toucan/core/scheduler.hpp>
#include "support/all_algos.hpp"

#include <toucan/sync/mutex.hpp>

#include <atomic>
#include <thread>

using namespace toucan;
using namespace std::chrono_literals;

template <typename Algo>
static void BM_SlowThread(benchmark::State& state) {
    static const auto kSmallSleepTime = 2ns;
    const size_t kThreads = state.range(0);
    const size_t kTasks = state.range(1);
    const size_t kIters = state.range(2);
    const auto kBigSleepTime = kSmallSleepTime * state.range(3);;

    const auto main_thread_id = std::this_thread::get_id();
    std::atomic<std::thread::id> slow_thread{main_thread_id};

    auto task = [&] {
        auto expected = main_thread_id;
        slow_thread.compare_exchange_strong(
            expected,
            std::this_thread::get_id()
        );
        auto slow_thread_local = slow_thread.load();
        for (size_t i = 0; i != kIters; ++i) {
            Yield();
            if (std::this_thread::get_id() == slow_thread_local) {
                std::this_thread::sleep_for(kBigSleepTime);
            } else {
                std::this_thread::sleep_for(kSmallSleepTime);
            }
        }
    };

    auto scheduler = Scheduler::Create<Algo>(kThreads);
    for (auto _ : state) {
        for (size_t i = 0; i != kTasks; ++i) {
            scheduler.Spawn(task);
        }
        scheduler.WaitAll();
    }
}

static void SlowThreadArgs(benchmark::internal::Benchmark* b) {
    // threads, tasks, iters, slowness
    // b->Args({4, 10, 100, 10});
    // b->Args({4, 100, 10, 10});
    // b->Args({4, 100, 100, 10});
    // b->Args({4, 10, 1000, 10});
    b->Args({4, 1000, 10, 100});
    // b->Args({4, 100, 100, 100});
}

BENCHMARK_WITH_ALL_ALGOS(SlowThread);

// Run the benchmark
BENCHMARK_MAIN();
