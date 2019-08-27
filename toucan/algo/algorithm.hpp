#pragma once

namespace toucan {
namespace algo {

class Algorithm {
  public:
    Algorithm() {
    }

    virtual ~Algorithm() {
    }

    virtual Fiber* PickNextFiber() = 0;
    virtual void Add(Fiber* fiber) = 0;  // Another name?
    virtual bool HasFibers() = 0;
};

}  // namespace algo
}  // namespace toucan
