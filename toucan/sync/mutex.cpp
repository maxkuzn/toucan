#include <toucan/sync/mutex.hpp>

#include <toucan/support/assert.hpp>
#include <toucan/core/scheduler.hpp>

namespace toucan {

Mutex::Mutex() {
}

Mutex::~Mutex() {
    ASSERT(wait_queue_.Empty(), "There are still waiting fibers");
}

void Mutex::Lock() {
    auto curr_fiber = GetCurrentFiber();
    ASSERT(owner_ != curr_fiber, "Already locked");

    Fiber* expected = nullptr;
    while (!owner_.compare_exchange_weak(expected, curr_fiber)) {
        wait_queue_.Wait();
        expected = nullptr;
    }
}

void Mutex::Unlock() {
    ASSERT(owner_ == GetCurrentFiber(), "Unlock can only owner");

    owner_.store(nullptr);
    wait_queue_.WakeOne();
}

bool Mutex::TryLock() {
    auto curr_fiber = GetCurrentFiber();
    ASSERT(owner_ != GetCurrentFiber(), "Already locked");

    Fiber* expected = nullptr;
    return owner_.compare_exchange_weak(expected, curr_fiber);
}
    

}  // namespace toucan

