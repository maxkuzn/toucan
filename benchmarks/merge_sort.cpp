#include <benchmark/benchmark.h>

#include <toucan/core/scheduler.hpp>
#include "support/all_algos.hpp"

#include <toucan/sync/condition_variable.hpp>

#include <random>

using namespace toucan;

std::vector<int> GenerateRandomVector(size_t size) {
    static std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    std::vector<int> v(size);
    for (auto& n : v) {
        n = dis(gen);
    }
    return v;
}

void Merge(std::vector<int>& data, std::vector<int>& buff,
           size_t from, size_t middle, size_t to) {
    size_t l = from;
    size_t r = middle;
    size_t buff_idx = from;
    while (l != middle && r != to) {
        if (data[l] < data[r]) {
            buff[buff_idx++] = data[l++];
        } else {
            buff[buff_idx++] = data[r++];
        }
    }
    while (l != middle) {
        buff[buff_idx++] = data[l++];
    }
    while (r != to) {
        buff[buff_idx++] = data[r++];
    }
    for (size_t i = from; i != to; ++i) {
        data[i] = buff[i];
    }
}

void Task(size_t from, size_t to,
        std::vector<int>& data, std::vector<int>& buff,
        size_t& ready, Mutex& mutex, ConditionVariable& cv) {
    if (to - from >= 2) {
        size_t my_ready = 0;
        Mutex my_mutex;
        ConditionVariable my_cv;
        size_t middle = from + (to - from) / 2;
        Spawn([&] {
            Task(from, middle, data, buff, my_ready, my_mutex, my_cv);
        });
        Spawn([&] {
            Task(middle, to, data, buff, my_ready, my_mutex, my_cv);
        });
        UniqueLock my_lock(my_mutex);
        my_cv.Wait(my_lock, [&] { return my_ready == 2; });
        Merge(data, buff, from, middle, to);
    }
    UniqueLock lock(mutex);
    ready++;
    cv.NotifyAll();
}

template <typename Algo>
static void BM_MergeSort(benchmark::State& state) {
    const size_t kThreads = state.range(0);
    const size_t kSize = state.range(1);

    auto scheduler = Scheduler::Create<Algo>(kThreads);
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> data = GenerateRandomVector(kSize);
        std::vector<int> buff(kSize);
        state.ResumeTiming();
        scheduler.Spawn([&] {
            size_t ready = 0;
            Mutex mutex;
            ConditionVariable cv;
            Spawn([&] {
                Task(0, kSize, data, buff, ready, mutex, cv);
            });
            UniqueLock lock(mutex);
            cv.Wait(lock, [&] { return ready == 1; });
        });
        scheduler.WaitAll();
    }
}

static void MergeSortArgs(benchmark::internal::Benchmark* b) {
    b->Args({1, 1024});
    b->Args({2, 1024});
    b->Args({4, 1024});
    b->Args({1, 1024 * 128});
    b->Args({2, 1024 * 128});
    b->Args({4, 1024 * 128});
}

BENCHMARK_WITH_ALL_ALGOS(MergeSort);

// Run the benchmark
BENCHMARK_MAIN();
