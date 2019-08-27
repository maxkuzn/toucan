#pragma once

#include <toucan/core/scheduler.hpp>

#include <queue>
#include <mutex>

namespace toucan {
namespace algo {

class FIFO : public Algorithm {
  public:
    FIFO() {
    }

    virtual ~FIFO() {
    }

  protected:
    virtual Fiber* PickNextFiber() final;
    virtual void Add(Fiber* fiber) final;
    virtual bool HasFibers() final;

  private:
    std::queue<Fiber*> ready_queue_;
    std::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan
