#pragma once

#include <atomic>
#include <toucan/core/api.hpp>

namespace toucan {
namespace testing {

class FiberBarrier {
  public:
    FiberBarrier(size_t fibers)
      : fibers_(fibers), count_(fibers) {
    }

    void PassThrough() {
        size_t epoch = epoch_.load();
        size_t curr_count = count_.fetch_sub(1) - 1;
        if (curr_count == 0) {
            count_.store(fibers_);
            ++epoch_;
        } else {
            while (epoch == epoch_.load()) {
                Yield();
            }
        }
    }

  private:
    const size_t fibers_;
    std::atomic<size_t> count_;
    std::atomic<size_t> epoch_{0};
};

}  // namespace testing
}  // namespace toucan

