#pragma once

#include <twist/stdlike/atomic.hpp>

namespace toucan {

class SpinLock {
  public:
    SpinLock() = default;
    ~SpinLock() = default;

    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;

    void Lock() {
        while (locked_.exchange(true)) {
            // do nothing
        }
    }

    void Unlock() {
        locked_.store(false);
    }

  private:
    twist::atomic<bool> locked_{false};
};

}  // namespace toucan

