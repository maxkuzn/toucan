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

    ~FIFO() {
    }

    void SetWorkersNum(size_t /*workers_num*/) override {
    }

    void SetupWorker() override {
    }

    Fiber* PickNextFiber() override;
    void Add(Fiber* fiber) override;

    void Shutdown() override {
    }

  private:
    std::queue<Fiber*> ready_queue_;
    twist::mutex mutex_;
};

}  // namespace algo
}  // namespace toucan
