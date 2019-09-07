#include <toucan/algo/work_stealing.hpp>

#include <toucan/core/scheduler.hpp>

#include <mutex>

namespace toucan {
namespace algo {

WorkStealing::~WorkStealing() {
    for (Data* d : workers_data_) {
        delete d;
    }
    workers_data_.clear();
}

void WorkStealing::SetWorkersNum(size_t workers_num) {
    workers_data_.resize(workers_num, nullptr);
}

void WorkStealing::SetupWorker() {
    size_t id = GetCurrentWorker()->id;
    workers_data_[id] = new Data();
    workers_data_[id]->gen.seed(id * 123 + 42);

    // wait for all workers
    ++next_id_;
    while (next_id_ != workers_data_.size()) {
        // do nothing
    }
}

Fiber* WorkStealing::PickNextFiber () {
    // Search in local queue
    size_t id = GetCurrentWorker()->id;
    Data* local_data = workers_data_[id];
    // Every 61 tick skip local storage
    if (local_data->count_fiber++ % 61 != 0) {
        std::unique_lock<twist::mutex> local_lock(local_data->mutex);
        auto& local_queue = local_data->queue;
        if (!local_queue.empty()) {
            auto fiber = local_queue.front();
            local_queue.pop();
            return fiber;
        }
        local_lock.unlock();
    }

    // Go to global queue
    std::unique_lock<twist::mutex> global_lock(global_mutex_);
    if (!global_queue_.empty()) {
        auto fiber = global_queue_.front();
        global_queue_.pop();
        return fiber;
    }
    global_lock.unlock();

    // Try to steal
    static const size_t kTries = 10;
    std::uniform_int_distribution<size_t> dis(0, workers_data_.size() - 1);
    for (size_t i = 0; i != kTries; ++i) {
        size_t victim_id = dis(local_data->gen);
        if (victim_id == id) {
            continue;
        }
        std::unique_lock<twist::mutex> lock(workers_data_[victim_id]->mutex);
        auto& victim_queue = workers_data_[victim_id]->queue;
        if (!victim_queue.empty()) {
            auto fiber = victim_queue.front();
            victim_queue.pop();
            return fiber;
        }
    }
    return nullptr;
}

void WorkStealing::Add(Fiber* fiber) {
    Worker* worker = GetCurrentWorker();
    if (worker) {
        Data* local_data = workers_data_[worker->id];
        std::unique_lock<twist::mutex> guard(local_data->mutex);
        local_data->queue.push(fiber);
    } else {
        std::unique_lock<twist::mutex> guard(global_mutex_);
        global_queue_.push(fiber);
    }
}

}  // namespace algo
}  // namespace toucan

