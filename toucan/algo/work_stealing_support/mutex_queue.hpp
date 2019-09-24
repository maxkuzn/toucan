#pragma once

#include <toucan/core/fiber.hpp>
#include <queue>
#include <twist/stdlike/mutex.hpp>

namespace toucan {
namespace algo {

class MutexQueue {
  public:
    MutexQueue() {
    }

    ~MutexQueue() {
    }

    void Put(Fiber* fiber, GlobalQueue&) {
        std::unique_lock<twist::mutex> lock(mutex_);
        data_.push(fiber);
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

    Fiber* GetFromGlobal(GlobalQueue& global_queue) {
        return global_queue.Get();
    }


    Fiber* Steal(MutexQueue& other) {
        return other.Get();
    }

  private:
    std::queue<Fiber*> data_;
    twist::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan

