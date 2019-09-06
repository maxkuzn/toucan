#include <toucan/sync/mutex.hpp>

#include <toucan/support/assert.hpp>
#include <toucan/core/scheduler.hpp>

namespace toucan {

Mutex::Mutex() {
    futex_.SetValue(0);
}

Mutex::~Mutex() {
    // ASSERT(futex_.Empty(), "There are still waiting fibers");
}

void Mutex::Lock() {
    auto curr_fiber = GetCurrentFiber();
    ASSERT(owner_ != curr_fiber, "Already locked");

    Fiber* expected = nullptr;
    while (!owner_.compare_exchange_weak(expected, curr_fiber)) {
        futex_.Wait(1);
        expected = nullptr;
    }
    futex_.SetValue(1);
}

void Mutex::Unlock() {
    ASSERT(owner_ == GetCurrentFiber(), "Unlock can only owner");
    
    futex_.SetValue(0);
    owner_.store(nullptr);
    futex_.WakeOne();
}

bool Mutex::TryLock() {
    auto curr_fiber = GetCurrentFiber();
    ASSERT(owner_ != GetCurrentFiber(), "Already locked");

    Fiber* expected = nullptr;
    if (owner_.compare_exchange_weak(expected, curr_fiber)) {
        futex_.SetValue(1);
        return true;
    }
    return false;
}

}  // namespace toucan

