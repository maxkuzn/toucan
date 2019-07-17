#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>

namespace toucan {

class Scheduler {
  public:
    Scheduler() : Scheduler(std::thread::hardware_concurrency()) {
    }

    Scheduler(size_t max_threads);
    ~Scheduler();

    void Submit(std::function<void()> task);

  private:
    void Run();

    const size_t max_threads_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> work_queue_;

    std::atomic<bool> terminate_{false};
    std::vector<std::thread> workers;
};

}  // namespace toucan

