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

void Fiber::GetOwnership() {
    size_t tries = 0;
    Worker* expected;
    expected = nullptr;
    Worker* curr_worker = GetCurrentWorker();
    while (!owner_.compare_exchange_weak(expected, curr_worker)) {
        expected = nullptr;
        // TODO: may be this_thread::yield()
    }
}

bool Fiber::IsOwner() const {
    return owner_ == GetCurrentWorker();
}

}  // namespace toucan

