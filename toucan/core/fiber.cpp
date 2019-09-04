#include <toucan/core/fiber.hpp>

#include <toucan/core/scheduler.hpp>

#include <toucan/support/assert.hpp>

namespace toucan {

Fiber* Fiber::CreateFiber(FiberRoutine routine) {
    Fiber* fiber = new Fiber();
    fiber->stack_ = FiberStack::Allocate();
    fiber->routine_ = std::move(routine);
    fiber->state_ = FiberState::Runnable;

    SetupRoutine(fiber);

    return fiber;
}

static void FiberMain() {
    Fiber* fiber = GetCurrentFiber();

    auto routine = fiber->Routine();
    try {
        routine();
    } catch (...) {
        throw std::runtime_error("Fiber routine ended with exception");
    }

    GetCurrentScheduler()->Terminate();

    throw std::runtime_error("Shouldn't be reached");
}

void Fiber::SetupRoutine(Fiber* fiber) {
    fiber->context_.Setup(fiber->stack_, FiberMain);
}

}  // namespace toucan

