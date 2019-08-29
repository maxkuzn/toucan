#include <toucan/core/scheduler.hpp>

#include <toucan/core/fiber.hpp>
#include <toucan/support/assert.hpp>
#include <thread>

namespace toucan {

static thread_local Worker* current_worker = nullptr;

// next functions avaliable only in worker threads

Worker* GetCurrentWorker() {
    ASSERT(current_worker, "Should be in worker");
    return current_worker;
}

static void SetCurrentWorker(Worker* worker) {
    current_worker = worker;
}

Scheduler* GetCurrentScheduler() {
    return GetCurrentWorker()->scheduler;
}

Fiber* GetCurrentFiber() {
    return GetCurrentWorker()->fiber;
}

static void SetCurrentFiber(Fiber* fiber) {
    GetCurrentWorker()->fiber = fiber;
}

////////////////////////////////////////////////////////////////////

Scheduler::Scheduler(std::shared_ptr<algo::IAlgorithm> algo, size_t workers_num) : algo_(algo) {
    if (workers_num == 0) {
        throw std::runtime_error("Workers num should be above zero");
    }
    algo_->SetWorkersNum(workers_num);
    workers_.reserve(workers_num);
    for (size_t i = 0; i != workers_num; ++i) {
        SpawnWorker();
    }
}

Scheduler::~Scheduler() {
    Shutdown();
}

void Scheduler::Spawn(FiberRoutine routine) {
    ++tasks_;
    Fiber* fiber = Fiber::CreateFiber(routine);
    fiber->SetState(FiberState::Runnable);
    algo_->Add(fiber);
}

void Scheduler::WaitAll() {
    std::unique_lock<twist::mutex> lock(wait_mutex_);
    wait_cv_.wait(lock, [&] {
        return tasks_.load() == 0;
    });
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
    if (--tasks_ == 0) {
        wait_cv_.notify_all();
    }
}

void Scheduler::Shutdown() {
    shutdown_.store(true);
    algo_->Shutdown();
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
    fiber->Context().SwitchTo(GetCurrentWorker()->context);
}

void Scheduler::Execute(Fiber* fiber) {
    fiber->GetOwnership();
    fiber->SetState(FiberState::Running);
    SetCurrentFiber(fiber);
    SwitchTo(fiber);
}

void Scheduler::Reschedule(Fiber* fiber) {
    ASSERT(fiber->IsOwner(), "Reschedule should only owner");
    SetCurrentFiber(nullptr);
    auto state = fiber->State();
    if (state == FiberState::Terminated) {
        Destroy(fiber);
    } else if (state == FiberState::Runnable) {
        fiber->ResetOwner();
        algo_->Add(fiber);
    } else if (state == FiberState::Suspended) {
        fiber->ResetOwner();
    } else if (state == FiberState::Running) {
        ASSERT(state != FiberState::Running, "Fiber running in reschedule");
    } else {
        ASSERT(false, "Unknown fiber state");
    }
}

void Scheduler::Suspend(SpinLock& sl) {
    Fiber* fiber = GetCurrentFiber();
    fiber->SetState(FiberState::Suspended);
    sl.unlock();
    SwitchToScheduler();
    sl.lock();
}

void Scheduler::WakeUp(Fiber* fiber) {
    fiber->GetOwnership();
    fiber->SetState(FiberState::Runnable);
    fiber->ResetOwner();
    algo_->Add(fiber);
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
    algo_->SetupWorker();
}

void Scheduler::WorkerLoop() {
    while (!shutdown_.load()) {
        Fiber* fiber = algo_->PickNextFiber();
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

