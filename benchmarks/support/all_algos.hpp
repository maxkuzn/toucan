#pragma once

#include <toucan/algo/global_fifo.hpp>
#include <toucan/algo/work_stealing.hpp>
#include <toucan/algo/work_stealing_support/mutex_queue.hpp>
#include <toucan/algo/work_stealing_support/lf_ab_queue.hpp>

#define BENCHMARK_WITH_ALGO(BenchmarkName, AlgoName)        \
BENCHMARK_TEMPLATE(BM_ ## BenchmarkName, AlgoName)          \
    ->Apply(BenchmarkName ## Args)                          \
    ->Unit(benchmark::kMillisecond)


typedef toucan::algo::WorkStealing<
    toucan::algo::MutexQueue
> WorkStealing_MutexQueue_MutexQueue;

typedef toucan::algo::WorkStealing<
    toucan::algo::LFABQueue<false, false>
> WorkStealing_MutexQueue_LFABQueue_ff;

typedef toucan::algo::WorkStealing<
    toucan::algo::LFABQueue<true, false>
> WorkStealing_MutexQueue_LFABQueue_tf;

typedef toucan::algo::WorkStealing<
    toucan::algo::LFABQueue<false, true>
> WorkStealing_MutexQueue_LFABQueue_ft;

typedef toucan::algo::WorkStealing<
    toucan::algo::LFABQueue<true, true>
> WorkStealing_MutexQueue_LFABQueue_tt;

#define BENCHMARK_WITH_ALL_ALGOS(BenchmarkName)             \
BENCHMARK_WITH_ALGO(BenchmarkName, algo::GlobalFIFO);       \
BENCHMARK_WITH_ALGO(BenchmarkName, WorkStealing_MutexQueue_MutexQueue); \
BENCHMARK_WITH_ALGO(BenchmarkName, WorkStealing_MutexQueue_LFABQueue_ff); \
BENCHMARK_WITH_ALGO(BenchmarkName, WorkStealing_MutexQueue_LFABQueue_tf); \
BENCHMARK_WITH_ALGO(BenchmarkName, WorkStealing_MutexQueue_LFABQueue_ft); \
BENCHMARK_WITH_ALGO(BenchmarkName, WorkStealing_MutexQueue_LFABQueue_tt)

