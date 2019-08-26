#include <toucan/core/scheduler.hpp>

#include <toucan/core/fiber.hpp>
#include <thread>

namespace toucan {


////////////////////////////////////////////////////////////////////

static thread_local Worker* current_worker = nullptr;

// next functions avaliable only in worker threads

Worker* GetCurrentWorker() {
    return current_worker;
}

static void SetCurrentWorker(Worker* worker) {
    current_worker = worker;
}

Scheduler* GetCurrentScheduler() {
    return current_worker->scheduler;
}

Fiber* GetCurrentFiber() {
    return current_worker->fiber;
}

static void SetCurrentFiber(Fiber* fiber) {
    current_worker->fiber = fiber;
}

////////////////////////////////////////////////////////////////////

Scheduler::Scheduler(size_t workers_count) {
    workers_.reserve(workers_count);
    for (size_t i = 0; i != workers_count; ++i) {
        SpawnWorker();
    }
}

Scheduler::~Scheduler() {
    Shutdown();
}

void Scheduler::Spawn(FiberRoutine routine) {
    Schedule(Fiber::CreateFiber(routine));
    started_.store(true);
}

void Scheduler::Yield() {
    Fiber* fiber = GetCurrentFiber();
    fiber->SetState(FiberState::Runnable);
    SwitchToScheduler();
}

void Scheduler::Terminate() {
    Fiber* fiber = GetCurrentFiber();
    fiber->SetState(FiberState::Terminated);
    SwitchToScheduler();
}

void Scheduler::Destroy(Fiber* fiber) {
    delete fiber;
}

void Scheduler::Shutdown() {
    started_.store(true);
    shutdown_.store(true);
    for (auto& worker : workers_) {
        worker.thread.join();
    }
}

void Scheduler::SwitchTo(Fiber* fiber) {
    auto& current_context = GetCurrentWorker()->context;
    current_context.SwitchTo(fiber->Context());
}

void Scheduler::SwitchToScheduler() {
    Fiber* fiber = GetCurrentFiber();
    SetCurrentFiber(nullptr);
    fiber->Context().SwitchTo(GetCurrentWorker()->context);
}

void Scheduler::Execute(Fiber* fiber) {
    auto worker = GetCurrentWorker();
    worker->fiber = fiber;
    fiber->SetState(FiberState::Running);
    SwitchTo(fiber);
}

////////////////////////////////////////////////////////////////////

void Scheduler::SpawnWorker() {
    workers_.emplace_back();
    Worker* worker = &workers_.back();
    worker->scheduler = this;
    worker->thread = std::thread([this, worker] { WorkerMain(worker); });
}

void Scheduler::WorkerMain(Worker* me) {
    WorkerSetup(me);
    WorkerLoop();
    WorkerFinalize();
}

void Scheduler::WorkerSetup(Worker* me) {
    me->scheduler = this;
    SetCurrentWorker(me);
    while (!started_.load()) {
    }
}

void Scheduler::WorkerLoop() {
    while (!shutdown_.load()) {
        Fiber* fiber = PickNextFiber();
        if (!fiber) {
            continue;
        }
        Execute(fiber);
        Reschedule(fiber);
    }
}

void Scheduler::WorkerFinalize() {
    SetCurrentWorker(nullptr);
}

}  // namespace toucan

