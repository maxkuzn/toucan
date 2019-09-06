#pragma once

#include <toucan/core/fiber.hpp>
#include <toucan/core/wait_queue.hpp>

namespace toucan {

class Mutex {
  public:
    Mutex();
    ~Mutex();

    Mutex& operator=(const Mutex&) = delete;

    void Lock();
    void Unlock();
    bool TryLock();

  private:
    twist::atomic<Fiber*> owner_{nullptr};
    WaitQueue wait_queue_;
};

}  // namespace toucan

