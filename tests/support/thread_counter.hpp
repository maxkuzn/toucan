#pragma once

#include <thread>
#include <mutex>
#include <unordered_set>

namespace toucan {
namespace testing {

class ThreadCounter {
  public:
    void Touch() {
        std::unique_lock<std::mutex> lock(mutex_);
        threads_.insert(std::this_thread::get_id());
    }

    size_t ThreadCount() {
        std::unique_lock<std::mutex> lock(mutex_);
        return threads_.size();
    }

  private:
    std::unordered_set<std::thread::id> threads_;
    std::mutex mutex_;
};

}  // namespace testing
}  // namespace toucan

