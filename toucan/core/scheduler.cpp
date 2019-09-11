#include <toucan/core/scheduler.hpp>

#include <toucan/core/fiber.hpp>
#include <toucan/support/assert.hpp>
#include <thread>

namespace toucan {

static thread_local Worker* current_worker = nullptr;

// next functions avaliable only in worker threads

Worker* GetCurrentWorker() {
    // ASSERT(current_worker, "Should be in worker");
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
        workers_num = 1;
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
    NotifyWorkers();
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

// Return with unlocked lock
void Scheduler::Suspend(SpinLock& sl) {
    GetCurrentFiber()->SetState(FiberState::Suspended);
    GetCurrentWorker()->lock = &sl;
    SwitchToScheduler();
}

void Scheduler::WakeUp(Fiber* fiber) {
    fiber->SetState(FiberState::Runnable);
    algo_->Add(fiber);
    NotifyWorkers();
}

void Scheduler::Shutdown() {
    shutdown_.store(true);
    NotifyWorkers();
    algo_->Shutdown();
    for (auto& worker : workers_) {
        worker.thread.join();
    }
}

void Scheduler::Execute(Fiber* fiber) {
    fiber->SetState(FiberState::Running);
    SetCurrentFiber(fiber);
    SwitchTo(fiber);
}

void Scheduler::Reschedule(Fiber* fiber) {
    SetCurrentFiber(nullptr);
    auto state = fiber->State();
    if (state == FiberState::Terminated) {
        Destroy(fiber);
    } else if (state == FiberState::Runnable) {
        algo_->Add(fiber);
        NotifyWorkers();
    } else if (state == FiberState::Suspended) {
        GetCurrentWorker()->Unlock();
    } else if (state == FiberState::Running) {
        ASSERT(state != FiberState::Running, "Fiber running in reschedule");
    } else {
        ASSERT(false, "Unknown fiber state");
    }
}

void Scheduler::Destroy(Fiber* fiber) {
    delete fiber;
    if (--tasks_ == 0) {
        std::unique_lock<twist::mutex> lock(wait_mutex_);
        wait_cv_.notify_all();
    }
}

void Scheduler::SuspendWorker() {
    std::unique_lock<twist::mutex> lock(workers_mutex_);
    workers_cv_.wait(lock, [this] { return has_new_fibers_ || shutdown_; });
    has_new_fibers_ = false;
}

void Scheduler::NotifyWorkers() {
    std::unique_lock<twist::mutex> lock(workers_mutex_);
    has_new_fibers_ = true;
    lock.unlock();
    workers_cv_.notify_all();
}

////////////////////////////////////////////////////////////////////

void Scheduler::SwitchTo(Fiber* fiber) {
    auto& current_context = GetCurrentWorker()->context;
    current_context.SwitchTo(fiber->Context());
}

void Scheduler::SwitchToScheduler() {
    Fiber* fiber = GetCurrentFiber();
    fiber->Context().SwitchTo(GetCurrentWorker()->context);
}

////////////////////////////////////////////////////////////////////

void Scheduler::SpawnWorker() {
    workers_.emplace_back();
    Worker* worker = &workers_.back();
    worker->scheduler = this;
    worker->id = workers_.size() - 1;
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
            SuspendWorker();
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

