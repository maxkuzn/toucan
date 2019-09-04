#pragma once

#include <toucan/core/fiber.hpp>
#include <toucan/core/scheduler.hpp>
#include <toucan/core/wait_queue.hpp>

#include <toucan/support/spinlock.hpp>
#include <toucan/support/assert.hpp>

#include <iostream>

namespace toucan {

class Mutex {
  private:
    struct Node {
        Node* next = nullptr;
        Fiber* fiber = nullptr;
    };

  public:
    Mutex() {
    }

    ~Mutex() {
    }

    void Lock() {
        while (locked_.exchange(true)) {
            wait_queue_.Wait();
        }
    }

    void Unlock() {
        locked_.store(false);
        wait_queue_.WakeOne();
    }

    bool TryLock() {
        return !locked_.exchange(true);
    }

  private:
    twist::atomic<bool> locked_{false};
    WaitQueue wait_queue_;
};

using mutex = Mutex;

}  // namespace toucan

