#include <toucan/algo/fifo.hpp>

namespace toucan {
namespace algo {

Fiber* FIFO::PickNextFiber() {
    std::unique_lock<twist::mutex> guard(mutex_);
    if (ready_queue_.empty()) {
        return nullptr;
    }
    Fiber* next = ready_queue_.front();
    ready_queue_.pop();
    return next;
}

void FIFO::Add(Fiber* fiber) {
    std::unique_lock<twist::mutex> guard(mutex_);
    ready_queue_.push(fiber);
}

bool FIFO::HasFibers() {
    std::unique_lock<twist::mutex> guard(mutex_);
    return !ready_queue_.empty();
}

}  // namespace algo
}  // namespace toucan
