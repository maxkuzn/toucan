#include <benchmark/benchmark.h>
#include <toucan/core/scheduler.hpp>
#include "support/all_algos.hpp"

#include <toucan/sync/mutex.hpp>

using namespace toucan;
using namespace std::chrono_literals;


/*
 * In that test we have two spawners that pin on different
 * threads and spawn tasks with different speed and amount
 */

template <typename Algo>
static void BM_DifferentSpawners(benchmark::State& state) {
    static const auto kSleepTime = 2ns;
    const size_t kThreads = state.range(0);
    const size_t kTasksFirst = state.range(1);
    const size_t kTasksSecond = state.range(2);
    const size_t kIters = state.range(3);

    auto task = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            std::this_thread::sleep_for(kSleepTime);
            Yield();
        }
    };

    const auto main_thread_id = std::this_thread::get_id();
    std::atomic<std::thread::id> picked_thread{main_thread_id};

    std::atomic<bool> ready{false};

    auto spawner = [&] (size_t kTasks) {
        auto expected = main_thread_id;
        if (!picked_thread.compare_exchange_strong(
                expected,
                std::this_thread::get_id())
        ) {
            while (picked_thread.load() == std::this_thread::get_id()) {
                Yield();
            }
            ready.store(true);
        } else {
            while (!ready) {
            }
        }
        for (size_t i = 0; i != kTasks; ++i) {
            Spawn(task);
        }
    };

    auto scheduler = Scheduler::Create<Algo>(kThreads);
    for (auto _ : state) {
        state.PauseTiming();
        scheduler.Spawn([&] {
            spawner(kTasksFirst);
        });
        scheduler.Spawn([&] {
            spawner(kTasksSecond);
        });
        while (!ready) {
        }
        state.ResumeTiming();
        scheduler.WaitAll();
    }
}

static void DifferentSpawnersArgs(benchmark::internal::Benchmark* b) {
    b->Args({8, 100, 10, 100});
    b->Args({8, 1000, 100, 100});
    b->Args({8, 10000, 100, 10});
}

BENCHMARK_WITH_ALL_ALGOS(DifferentSpawners);

// Run the benchmark
BENCHMARK_MAIN();
