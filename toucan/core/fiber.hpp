#pragma once

#include <toucan/core/context.hpp>
#include <toucan/core/stack.hpp>

#include <toucan/support/intrusive_list_node.hpp>

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

class Fiber : public IntrusiveListNode<Fiber> {
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
    twist::atomic<FiberState> state_;
    FiberRoutine routine_;
};

}  // namespace toucan

