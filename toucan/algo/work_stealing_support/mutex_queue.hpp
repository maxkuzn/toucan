#pragma once

#include <toucan/core/fiber.hpp>
#include <queue>
#include <twist/stdlike/mutex.hpp>

namespace toucan {
namespace algo {


template <bool kStealOnlyOne, bool kGetOneFromGlob>
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
        if (kGetOneFromGlob) {
            return global_queue.Get();
        }
        global_queue.Lock();
        if (global_queue.SizeWithoutLock() == 0) {
            global_queue.Unlock();
            return nullptr;
        }
        std::unique_lock<twist::mutex> lock(mutex_);
        while (global_queue.SizeWithoutLock() > 0) {
            data_.push(global_queue.GetWithoutLock());
        }
        Fiber* fiber = data_.front();
        data_.pop();
        lock.unlock();
        global_queue.Unlock();
        return fiber;
    }


    Fiber* Steal(MutexQueue& other) {
        if (kStealOnlyOne) {
            return other.Get();
        }
        auto& mutex1 = this < &other ? mutex_ : other.mutex_;
        auto& mutex2 = this < &other ? other.mutex_ : mutex_;
        std::unique_lock<twist::mutex> lock1(mutex1);
        std::unique_lock<twist::mutex> lock2(mutex2);
        while (other.data_.size() > data_.size()) {
            data_.push(other.data_.front());
            other.data_.pop();
        }
        if (data_.empty()) {
            return nullptr;
        }
        auto fiber = data_.front();
        data_.pop();
        return fiber;
    }

  private:
    std::queue<Fiber*> data_;
    twist::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan

