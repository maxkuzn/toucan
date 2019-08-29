#pragma once

#include <toucan/core/context.hpp>
#include <toucan/core/stack.hpp>

#include <twist/stdlike/atomic.hpp>

#include <functional>

namespace toucan {

using FiberRoutine = std::function<void()>;

struct Worker;

enum class FiberState {
    Runnable,
    Running,
    Suspended,
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

    void GetOwnership();
    bool IsOwner() const;
    void ResetOwner() {
        owner_.store(nullptr);
    }


    static Fiber* CreateFiber(FiberRoutine routine);
    static void SetupRoutine(Fiber* fiber);

  private:
    FiberStack stack_;
    ExecutionContext context_;
    twist::atomic<FiberState> state_;
    twist::atomic<Worker*> owner_{nullptr};
    FiberRoutine routine_;
};

}  // namespace toucan

