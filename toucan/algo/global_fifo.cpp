#include <toucan/algo/global_fifo.hpp>

namespace toucan {
namespace algo {

Fiber* GlobalFIFO::PickNextFiber() {
    std::unique_lock<twist::mutex> guard(mutex_);
    if (ready_queue_.empty()) {
        return nullptr;
    }
    Fiber* next = ready_queue_.front();
    ready_queue_.pop();
    return next;
}

void GlobalFIFO::Add(Fiber* fiber) {
    std::unique_lock<twist::mutex> guard(mutex_);
    ready_queue_.push(fiber);
}

}  // namespace algo
}  // namespace toucan
