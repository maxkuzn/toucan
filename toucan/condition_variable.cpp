#include <toucan/condition_variable.hpp>

namespace toucan {

ConditionVariable::ConditionVariable() {
    // not implemented
}

void ConditionVariable::Wait(std::unique_lock<Mutex>& lock) {
    // not implemented
}

template <typename Predicate>
void ConditionVariable::Wait(std::unique_lock<Mutex>& lock, Predicate pred) {
    // not implemented
}

void ConditionVariable::NotifyOne() {
    // not implemented
}

void ConditionVariable::NotifyAll() {
    // not implemented
}

}  // namespace toucan
