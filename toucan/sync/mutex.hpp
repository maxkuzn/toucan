#pragma once

#include <toucan/core/fiber.hpp>
#include <toucan/core/scheduler.hpp>
#include <toucan/core/wait_queue.hpp>

#include <toucan/support/assert.hpp>

namespace toucan {

class Mutex {
  public:
    Mutex() {
    }

    ~Mutex() {
    }

    void Lock() {
        auto curr_fiber = GetCurrentFiber();
        ASSERT(owner_ != curr_fiber, "Already locked");

        Fiber* expected = nullptr;
        while (!owner_.compare_exchange_weak(expected, curr_fiber)) {
            wait_queue_.Wait();
            expected = nullptr;
        }
    }

    void Unlock() {
        ASSERT(owner_ == GetCurrentFiber(), "Unlock can only owner");

        owner_.store(nullptr);
        wait_queue_.WakeOne();
    }

    bool TryLock() {
        auto curr_fiber = GetCurrentFiber();
        ASSERT(owner_ != GetCurrentFiber(), "Already locked");

        Fiber* expected = nullptr;
        return owner_.compare_exchange_weak(expected, curr_fiber);
    }

  private:
    twist::atomic<Fiber*> owner_{nullptr};
    WaitQueue wait_queue_;
};

}  // namespace toucan

