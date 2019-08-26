#pragma once

#include <toucan/core/scheduler.hpp>

#include <queue>
#include <mutex>

namespace toucan {
namespace algo {

class FIFOScheduler : public Scheduler {
  public:
    FIFOScheduler() {
    }

    FIFOScheduler(size_t workers_count) : Scheduler(workers_count) {
    }

    virtual ~FIFOScheduler() {
    }

  protected:
    virtual Fiber* PickNextFiber() final;
    virtual void Schedule(Fiber* fiber) final;
    virtual void Reschedule(Fiber* fiber) final;

  private:
    std::queue<Fiber*> ready_queue_;
    std::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan
