#include <benchmark/benchmark.h>

#include <toucan/core/scheduler.hpp>
#include <toucan/algo/fifo.hpp>
#include <toucan/algo/work_stealing.hpp>

using namespace toucan;

template <typename Algo>
static void BM_Func(benchmark::State& state) {
    static const size_t kTasks = 100;
    static const size_t kIters = 100;
    std::atomic<size_t> count{0};

    auto task = [&] {
        for (size_t i = 0; i != kIters; ++i) {
            Yield();
            count++;
        }
    };

    auto scheduler = Scheduler::Create<Algo>(4);
    for (auto _ : state) {
        for (size_t i = 0; i != kTasks; ++i) {
            scheduler.Spawn(task);
        }
        scheduler.WaitAll();
    }
}
BENCHMARK_TEMPLATE(BM_Func, algo::FIFO);
BENCHMARK_TEMPLATE(BM_Func, algo::WorkStealing);

// Run the benchmark
BENCHMARK_MAIN();
