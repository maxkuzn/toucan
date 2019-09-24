#pragma once

#include <toucan/core/fiber.hpp>

namespace toucan {
namespace algo {

// lock-free array-based queue

class LFABQueue {
  public:
    LFABQueue() {
    }

    ~LFABQueue() {
    }

    // just put if it possible
    // Maybe: put half of fibers to global
    bool Put(Fiber* fiber) {
        auto h = head_.load();
        auto t = tail_.load();
        if (t - h < kSize) {
            data_[t % kSize] = fiber;
            ++tail_;
            return true;
        } else {
            return false;
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

    // steal just 1 fiber
    // Maybe: steal half of fibers
    Fiber* Steal() {
        return Get();
    }

  private:
    static const size_t kSize = 256;
    Fiber* data_[kSize];
    twist::atomic<size_t> head_{0};
    twist::atomic<size_t> tail_{0};
};

}  // namespace algo
}  // namespace toucan

