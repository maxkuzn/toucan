#pragma once

#include <toucan/core/fiber.hpp>
#include <toucan/sync/mutex.hpp>
#include <toucan/sync/condition_variable.hpp>

#include <list>

namespace toucan {

template <typename T>
class Channel {
  public:
    Channel() {
    }

    ~Channel() {
    }

    Channel& operator=(const Channel&) = delete;

    void Put(const T& elem) {
        UniqueLock lock(mutex_);
        queue_.push_back(elem);
        cv_.NotifyOne();
    }

    bool Get(T& elem, bool wait = true) {
        UniqueLock lock(mutex_);
        if (wait) {
            cv_.Wait(lock, [&] { return closed_ || !queue_.empty(); });
        }
        if (queue_.empty()) {
            return false;
        }
        elem = queue_.front();
        queue_.pop_front();
        return true;
    }

    void Close() {
        UniqueLock lock(mutex_);
        closed_ = true;
        cv_.NotifyAll();
    }

    bool IsClosed() {
        UniqueLock lock(mutex_);
        return closed_;
    }

  private:
    bool closed_ = false;
    std::list<T> queue_;
    Mutex mutex_;
    ConditionVariable cv_;
};

}  // namespace toucan

