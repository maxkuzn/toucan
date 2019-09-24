#pragma once

#include <toucan/algo/algorithm.hpp>
#include <toucan/core/scheduler.hpp>

#include <random>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/atomic.hpp>

#include <toucan/algo/work_stealing_support/global_queue.hpp>

namespace toucan {
namespace algo {

template <typename LocalQueue>
class WorkStealing : public IAlgorithm {
  private:
    struct Data {
        LocalQueue queue;
        std::mt19937 gen;
        size_t count_fiber = 0;
    };

  public:
    WorkStealing() {
    }

    ~WorkStealing() {
        for (Data* d : workers_data_) {
            delete d;
        }
        workers_data_.clear();
    }

    void SetWorkersNum(size_t workers_num) override {
        workers_data_.resize(workers_num, nullptr);
    }

    void SetupWorker() override {
        size_t id = GetCurrentWorker()->id;
        workers_data_[id] = new Data();
        workers_data_[id]->gen.seed(id * 123 + 42);

        // wait for all workers
        ++counter_;
        while (counter_ != workers_data_.size()) {
            // do nothing
        }
    }

    Fiber* PickNextFiber() override {
        size_t id = GetCurrentWorker()->id;
        Data* local_data = workers_data_[id];
        Fiber* fiber;

        // Every 61 tick first search in global queue
        if (local_data->count_fiber++ % 61 == 0) {
            fiber = local_data->queue.GetFromGlobal(global_queue_);
            if (fiber) {
                return fiber;
            }
        }

        // Search in local queue
        fiber = local_data->queue.Get();
        if (fiber) {
            return fiber;
        }

        // Go to global queue
        fiber = local_data->queue.GetFromGlobal(global_queue_);
        if (fiber) {
            return fiber;
        }

        // Try to steal
        const size_t kTries = 2 * workers_data_.size();
        std::uniform_int_distribution<size_t> dis(0, workers_data_.size() - 1);
        for (size_t i = 0; i != kTries; ++i) {
            size_t victim_id = dis(local_data->gen);
            if (victim_id == id) {
                continue;
            }
            auto& victim_queue = workers_data_[victim_id]->queue;
            fiber = local_data->queue.Steal(victim_queue);
            if (fiber) {
                return fiber;
            }
        }
        return nullptr;
    }

    void Add(Fiber* fiber) override {
        Worker* worker = GetCurrentWorker();
        if (worker) {
            Data* local_data = workers_data_[worker->id];
            local_data->queue.Put(fiber, global_queue_);
        } else {
            global_queue_.Put(fiber);
        }
    }

    void Shutdown() override {
    }

  private:
    twist::atomic<size_t> counter_{0};
    std::vector<Data*> workers_data_;
    GlobalQueue global_queue_;
};

}  // namespace algo
}  // namespace toucan

