#pragma once

#include <toucan/sync/unique_lock.hpp>
#include <toucan/sync/futex.hpp>

#include <toucan/support/spinlock.hpp>

namespace toucan {

class ConditionVariable {
  public:
    ConditionVariable();
    ~ConditionVariable();

    ConditionVariable& operator=(const ConditionVariable&) = delete;

    void Wait(UniqueLock& lock);

    template <typename Pred>
    void Wait(UniqueLock& lock, Pred pred) {
        while (!pred()) {
            Wait(lock);
        }
    }

    void NotifyOne();
    void NotifyAll();

  private:
    SpinLock sl_;
    Futex futex_;
};

}  // namespace toucan

