#pragma once

#include <toucan/core/fiber.hpp>
#include <toucan/sync/futex.hpp>

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
    Futex futex_;
};

}  // namespace toucan

