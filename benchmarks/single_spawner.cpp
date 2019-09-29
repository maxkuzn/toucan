#include <benchmark/benchmark.h>

#include <toucan/core/scheduler.hpp>
#include "support/all_algos.hpp"

#include <toucan/sync/mutex.hpp>

using namespace toucan;
using namespace std::chrono_literals;

template <typename Algo>
static void BM_SingleSpawner(benchmark::State& state) {
    static const auto kSleepTime = 2ns;
    const size_t kThreads = state.range(0);
    const size_t kTasks = state.range(1);
    const size_t kIters = state.range(2);
    const bool kSpawnerYield = (state.range(3) != 0);

    auto task = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            std::this_thread::sleep_for(kSleepTime);
            Yield();
        }
    };

    auto spawner = [&] {
        for (size_t i = 0; i != kTasks; ++i) {
            Spawn(task);
            if (kSpawnerYield) {
                Yield();
            }
        }
    };

    auto scheduler = Scheduler::Create<Algo>(kThreads);
    for (auto _ : state) {
        scheduler.Spawn(spawner);
        scheduler.WaitAll();
    }
}

static void SingleSpawnerArgs(benchmark::internal::Benchmark* b) {
    b->Args({4, 1000, 10, 0});
    //b->Args({4, 1000, 10, 1});
}

BENCHMARK_WITH_ALL_ALGOS(SingleSpawner);

// Run the benchmark
BENCHMARK_MAIN();
