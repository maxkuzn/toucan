#pragma once

#include <mutex>
#include <toucan/mutex.hpp>

namespace toucan {

class ConditionVariable {
  public:
    ConditionVariable();

    void Wait(std::unique_lock<Mutex>& lock);

    template <typename Predicate>
    void Wait(std::unique_lock<Mutex>& lock, Predicate pred);

    void NotifyOne();
    void NotifyAll();
};

}  // namespace toucan

