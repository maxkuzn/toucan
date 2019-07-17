#include <toucan/scheduler.hpp>

namespace toucan {

Scheduler::Scheduler(size_t max_threads) : max_threads_(max_threads) {
    for (size_t i = 0; i != max_threads_; ++i) {
        workers.emplace_back([&] {
            Run();
        });
    }
}

Scheduler::Scheduler() : Scheduler(std::thread::hardware_concurrency()) {
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

void Scheduler::WaitAll() {
    std::unique_lock<std::mutex> guard(mtx_);
    cv_wait_.wait(guard, [&] { return work_queue_.empty(); });
}

void Scheduler::Run() {
    while (!terminate_) {
        std::unique_lock<std::mutex> guard(mtx_);
        ++idle_threads_;
        if (work_queue_.empty() && idle_threads_ == max_threads_) {
            cv_wait_.notify_all();
        }
        cv_.wait(guard, [&] {
            return !work_queue_.empty() || terminate_;
        });
        if (terminate_) {
            break;
        }
        auto task = work_queue_.front();
        work_queue_.pop();
        if (work_queue_.empty() && idle_threads_ == max_threads_) {
            cv_wait_.notify_all();
        }
        --idle_threads_;
        guard.unlock();
        task();
    }
}

}  // namespace toucan

