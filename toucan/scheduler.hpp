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
    Scheduler();
    Scheduler(size_t max_threads);
    ~Scheduler();

    void Submit(std::function<void()> task);
    void WaitAll();

  private:
    void Run();

    const size_t max_threads_;
    size_t idle_threads_ = 0;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::condition_variable cv_wait_;
    std::queue<std::function<void()>> work_queue_;

    std::atomic<bool> terminate_{false};
    std::vector<std::thread> workers;
};

}  // namespace toucan

