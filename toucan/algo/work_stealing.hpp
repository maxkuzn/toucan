#pragma once

#include <toucan/algo/algorithm.hpp>

#include <random>
#include <queue>
#include <vector>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/atomic.hpp>

namespace toucan {
namespace algo {

class WorkStealing : public IAlgorithm {
  private:
    struct Data {
        twist::mutex mutex;
        std::queue<Fiber*> queue;
        std::mt19937 gen;
        size_t count_fiber = 0;
    };

  public:
    WorkStealing() {
    }

    ~WorkStealing();

    void SetWorkersNum(size_t workers_num) override;

    void SetupWorker() override;

    Fiber* PickNextFiber() override;
    void Add(Fiber* fiber) override;

    void Shutdown() override {
    }

  private:
    twist::atomic<size_t> next_id_{0};
    std::vector<Data*> workers_data_;

    twist::mutex global_mutex_;
    std::queue<Fiber*> global_queue_;
};

}  // namespace algo
}  // namespace toucan

