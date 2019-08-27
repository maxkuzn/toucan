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

    virtual Fiber* PickNextFiber() = 0;
    virtual void Add(Fiber* fiber) = 0;  // Another name?
    virtual bool HasFibers() = 0;
};

}  // namespace algo
}  // namespace toucan
