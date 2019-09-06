#include <toucan/sync/condition_variable.hpp>

#include <toucan/support/assert.hpp>

namespace toucan {

ConditionVariable::ConditionVariable() {
    futex_.SetValue(0);
}

ConditionVariable::~ConditionVariable() {
    // ASSERT(wait_queue_.Empty(), "There are fibers that still waiting");
}

void ConditionVariable::Wait(UniqueLock& lock) {
    auto value = futex_.GetValue();
    lock.Unlock();
    futex_.Wait(value);
    lock.Lock();
}

void ConditionVariable::NotifyOne() {
    futex_.IncrValue();
    futex_.WakeOne();
}

void ConditionVariable::NotifyAll() {
    futex_.IncrValue();
    futex_.WakeAll();
}

}  // namespace toucan

