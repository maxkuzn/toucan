#pragma once

#include <toucan/sync/mutex.hpp>

namespace toucan {

class UniqueLock {
  public:
    UniqueLock(Mutex& mutex);
    ~UniqueLock();

    void Lock();
    void Unlock();

  private:
    bool locked_ = false;
    Mutex& mutex_;
};

}  // namespace toucan

