#pragma once

#include <toucan/core/fiber.hpp>
#include <queue>
#include <twist/stdlike/mutex.hpp>

namespace toucan {
namespace algo {

class GlobalQueue {
  public:
    GlobalQueue() {
    }

    void PutWithoutLock(Fiber* fiber) {
        queue_.push(fiber);
    }

    void Put(Fiber* fiber) {
        std::unique_lock<twist::mutex> lock(mutex_);
        PutWithoutLock(fiber);
    }

    Fiber* GetWithoutLock() {
        if (queue_.empty()) {
            return nullptr;
        }
        Fiber* fiber = queue_.front();
        queue_.pop();
        return fiber;
    }

    Fiber* Get() {
        std::unique_lock<twist::mutex> lock(mutex_);
        return GetWithoutLock();
    }

    size_t SizeWithoutLock() {
        return queue_.size();
    }

    size_t Size() {
        std::unique_lock<twist::mutex> lock(mutex_);
        return SizeWithoutLock();
    }

    void Lock() {
        mutex_.lock();
    }

    void Unlock() {
        mutex_.unlock();
    }

  private:
    std::queue<Fiber*> queue_;
    twist::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan
