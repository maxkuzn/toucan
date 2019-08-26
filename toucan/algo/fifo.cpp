#include <toucan/algo/fifo.hpp>

namespace toucan {
namespace algo {

Fiber* FIFOScheduler::PickNextFiber() {
    std::unique_lock<std::mutex> guard(mutex_);
    if (ready_queue_.empty()) {
        return nullptr;
    }
    Fiber* next = ready_queue_.front();
    ready_queue_.pop();
    return next;
}

void FIFOScheduler::Schedule(Fiber* fiber) {
    std::unique_lock<std::mutex> guard(mutex_);
    ready_queue_.push(fiber);
}

void FIFOScheduler::Reschedule(Fiber* fiber) {
    switch (fiber->State()) {
    case FiberState::Runnable:
        {
            std::unique_lock<std::mutex> guard(mutex_);
            ready_queue_.push(fiber);
        }
        break;
    case FiberState::Terminated:
        Destroy(fiber);
        break;
    default:
        throw std::runtime_error("Unknown fiber state");
        break;
    }
}



}  // namespace algo
}  // namespace toucan
