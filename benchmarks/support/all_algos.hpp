#pragma once

#include <toucan/algo/global_fifo.hpp>
#include <toucan/algo/work_stealing.hpp>
#include <toucan/algo/work_stealing_support/mutex_queue.hpp>

#define BENCHMARK_WITH_ALGO(BenchmarkName, AlgoName)        \
BENCHMARK_TEMPLATE(BM_ ## BenchmarkName, AlgoName)          \
    ->Apply(BenchmarkName ## Args)                          \
    ->Unit(benchmark::kMillisecond)


typedef toucan::algo::WorkStealing<
    toucan::algo::MutexQueue,
    toucan::algo::MutexQueue
> WorkStealing_MutexQueue_MutexQueue;

#define BENCHMARK_WITH_ALL_ALGOS(BenchmarkName)             \
BENCHMARK_WITH_ALGO(BenchmarkName, algo::GlobalFIFO);       \
BENCHMARK_WITH_ALGO(BenchmarkName, WorkStealing_MutexQueue_MutexQueue)

