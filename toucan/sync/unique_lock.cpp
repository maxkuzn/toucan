#include <toucan/sync/unique_lock.hpp>

#include <toucan/support/assert.hpp>

namespace toucan {

UniqueLock::UniqueLock(Mutex& mutex) : mutex_(mutex) {
    Lock();
}

UniqueLock::~UniqueLock() {
    if (locked_) {
        Unlock();
    }
}

void UniqueLock::Lock() {
    ASSERT(!locked_, "Shouldn't be locked before");
    mutex_.Lock();
    locked_ = true;
}

void UniqueLock::Unlock() {
    ASSERT(locked_, "Should be locked before");
    mutex_.Unlock();
    locked_ = false;
}

}  // namespace toucan
