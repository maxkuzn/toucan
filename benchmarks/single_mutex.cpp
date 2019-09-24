#include <benchmark/benchmark.h>

#include <toucan/core/scheduler.hpp>
#include <toucan/algo/global_fifo.hpp>
#include <toucan/algo/work_stealing.hpp>
#include "support/all_algos.hpp"

#include <toucan/sync/mutex.hpp>

using namespace toucan;
using namespace std::chrono_literals;

template <typename Algo>
static void BM_SingleMutex(benchmark::State& state) {
    static const auto kCriticalSleepTime = 2ns;
    const size_t kThreads = state.range(0);
    const size_t kTasks = state.range(1);
    const size_t kIters = state.range(2);

    Mutex mutex;
    
    auto task = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            mutex.Lock();
            std::this_thread::sleep_for(kCriticalSleepTime);
            mutex.Unlock();
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

static void SingleMutexArgs(benchmark::internal::Benchmark* b) {
    b->Args({4, 10, 1000});
    b->Args({4, 100, 100});
    b->Args({4, 100, 1000});
    b->Args({4, 1000, 10});
    b->Args({4, 1000, 100});
}

// BENCHMARK_TEMPLATE(BM_SingleMutex, algo::GlobalFIFO)->Apply(SingleMutexArgs)->Unit(benchmark::kMillisecond);
// BENCHMARK_TEMPLATE(BM_SingleMutex, algo::WorkStealing)->Apply(SingleMutexArgs)->Unit(benchmark::kMillisecond);
BENCHMARK_WITH_ALL_ALGOS(SingleMutex);

// Run the benchmark
BENCHMARK_MAIN();
