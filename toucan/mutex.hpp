#pragma once

namespace toucan {

class Mutex {
  public:
    Mutex();

    void Lock();
    void Unlock();

    // for unique_lock
    void lock() {
        Lock();
    }

    void unlock() {
        Unlock();
    }
};

}  // namespace toucan

