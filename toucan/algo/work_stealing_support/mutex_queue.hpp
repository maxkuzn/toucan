#pragma once

#include <toucan/core/fiber.hpp>

namespace toucan {
namespace algo {

class MutexQueue {
  public:
    MutexQueue() {
    }

    ~MutexQueue() {
    }

    bool Put(Fiber* fiber) {
        std::unique_lock<twist::mutex> lock(mutex_);
        data_.push(fiber);
        return true;
    }

    Fiber* Get() {
        std::unique_lock<twist::mutex> lock(mutex_);
        if (data_.empty()) {
            return nullptr;
        }
        auto fiber = data_.front();
        data_.pop();
        return fiber;
    }

    Fiber* Steal() {
        return Get();
    }

  private:
    std::queue<Fiber*> data_;
    twist::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan

