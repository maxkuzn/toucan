#pragma once

#include <toucan/core/fiber.hpp>

namespace toucan {

namespace algo {

class IAlgorithm {
  public:
    IAlgorithm() {
    }

    virtual ~IAlgorithm() {
    }

    IAlgorithm(const IAlgorithm&) = delete;
    IAlgorithm(IAlgorithm&&) = delete;
    IAlgorithm& operator=(const IAlgorithm&) = delete;
    IAlgorithm& operator=(IAlgorithm&&) = delete;

    virtual void SetWorkersNum(size_t workers_num) = 0;
    virtual void SetupWorker() = 0;

    virtual Fiber* PickNextFiber() = 0;
    virtual void Add(Fiber* fiber) = 0;  // Another name?

    virtual bool HasFibers() = 0;

    virtual void Shutdown() = 0;
};

}  // namespace algo
}  // namespace toucan
