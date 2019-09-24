#pragma once

#include <toucan/core/fiber.hpp>
#include <toucan/algo/work_stealing_support/global_queue.hpp>

namespace toucan {
namespace algo {

// lock-free array-based queue

template <bool kPutOneToGlob, bool kStealOnlyOne>
class LFABQueue {
  public:
    LFABQueue() {
    }

    ~LFABQueue() {
    }

    void Put(Fiber* fiber, GlobalQueue& global_queue) {
        for (;;) {
            auto h = head_.load();
            auto t = tail_.load();
            if (t - h < kSize) {
                data_[t % kSize] = fiber;
                ++tail_;
                return;
            } else if (kPutOneToGlob) {
                global_queue.Put(fiber);
                return;
            } else {
                // Put half
                global_queue.Lock();
                if (!head_.compare_exchange_weak(h, h + (t - h) / 2)) {
                    continue;
                }
                for (size_t i = h; i != h + (t - h) / 2; ++i) {
                    global_queue.PutWithoutLock(data_[i % kSize]);
                }
                global_queue.Unlock();
            }
        }
    }

    // get while not empty
    Fiber* Get() {
        for (;;) {
            auto h = head_.load();
            auto t = tail_.load();
            if (h == t) {
                return nullptr;
            }
            Fiber* fiber = data_[h % kSize];
            if (head_.compare_exchange_weak(h, h + 1)) {
                return fiber;
            }
        }
    }

    Fiber* GetFromGlobal(GlobalQueue& global_queue) {
        return global_queue.Get();
    }

    Fiber* Steal(LFABQueue& other) {
        if (kStealOnlyOne) {
            return other.Get();
        }
        // steal half
        size_t n;
        auto t = tail_.load();
        for (;;) {
            auto other_h = other.head_.load();
            auto other_t = other.tail_.load();
            n = other_t - other_h;
            n -= n/2;
            if (n == 0) {
                return nullptr;
            }
            for (size_t i = 0; i != n; ++i) {
                data_[(t + i) % kSize] = other.data_[(other_h + i) % kSize];
            }
            if (other.head_.compare_exchange_weak(other_h, other_h + n)) {
                break;
            }
        }
        --n;
        Fiber* res = data_[(t + n) % kSize];
        if (n == 0) {
            return res;
        }
        auto h = head_.load();
        if (t - h + n >= kSize) {
            throw std::runtime_error("Runq overflow");
        }
        tail_.store(t + n);
        return res;
    }

  private:
    static const size_t kSize = 256;
    Fiber* data_[kSize];
    twist::atomic<size_t> head_{0};
    twist::atomic<size_t> tail_{0};
};

}  // namespace algo
}  // namespace toucan

