#include <toucan/scheduler.hpp>

#include <iostream>

namespace toucan {

Scheduler::Scheduler(size_t max_threads) : max_threads_(max_threads) {
    for (size_t i = 0; i != max_threads_; ++i) {
        workers.emplace_back([&] {
            Run();
        });
    }
}

Scheduler::~Scheduler() {
    terminate_ = true;
    cv_.notify_all();
    for (auto&& w : workers) {
        w.join();
    }
}

void Scheduler::Submit(std::function<void()> task) {
    std::unique_lock<std::mutex> guard(mtx_);
    work_queue_.push(task);
    cv_.notify_one();
}

void Scheduler::Run() {
    std::cout << "Another thread\n";
    while (!terminate_) {
        std::unique_lock<std::mutex> guard(mtx_);
        cv_.wait(guard, [&] {
            return !work_queue_.empty() || terminate_;
        });
        if (terminate_) {
            break;
        }
        auto task = work_queue_.front();
        work_queue_.pop();
        guard.unlock();
        task();
    }
}

}  // namespace toucan

