#pragma once

namespace toucan {
namespace algo {

class Algorithm {
  public:
    Algorithm() {
    }

    virtual ~Algorithm() {
    }

    Algorithm(const Algorithm&) = delete;
    Algorithm(Algorithm&&) = delete;
    Algorithm& operator=(const Algorithm&) = delete;
    Algorithm& operator=(Algorithm&&) = delete;

    virtual void SetWorkersNum(size_t workers_num) = 0;
    virtual void SetupWorker() = 0;

    virtual Fiber* PickNextFiber() = 0;
    virtual void Add(Fiber* fiber) = 0;  // Another name?

    virtual bool HasFibers() = 0;

    virtual void Shutdown() = 0;
};

}  // namespace algo
}  // namespace toucan
