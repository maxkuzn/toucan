#pragma once

#include <toucan/core/scheduler.hpp>

#include <queue>
#include <twist/stdlike/mutex.hpp>

namespace toucan {
namespace algo {

class FIFO : public IAlgorithm {
  public:
    FIFO() {
    }

    virtual ~FIFO() {
    }

    virtual void SetWorkersNum(size_t /*workers_num*/) {
    }

    virtual void SetupWorker() {
    }

    virtual Fiber* PickNextFiber() final;
    virtual void Add(Fiber* fiber) final;
    virtual bool HasFibers() final;

    virtual void Shutdown() final {
    }

  private:
    std::queue<Fiber*> ready_queue_;
    twist::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan
