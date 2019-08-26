#pragma once

#include <toucan/core/context.hpp>
#include <toucan/core/stack.hpp>
#include <toucan/core/api.hpp>

namespace toucan {

enum class FiberState {
    Runnable,
    Running,
    Terminated
};

class Fiber {
  public:
    FiberState State() const {
        return state_;
    }

    void SetState(FiberState target) {
        state_ = target;
    }

    FiberRoutine Routine() const {
        return routine_;
    }

    ExecutionContext& Context() {
        return context_;
    }

    static Fiber* CreateFiber(FiberRoutine routine);
    static void SetupRoutine(Fiber* fiber);

  private:
    FiberStack stack_;
    ExecutionContext context_;
    FiberState state_;
    FiberRoutine routine_;
};

}  // namespace toucan

