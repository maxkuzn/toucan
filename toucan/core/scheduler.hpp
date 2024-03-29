#pragma once

#include <toucan/core/api.hpp>
#include <toucan/core/fiber.hpp>

#include <toucan/algo/algorithm.hpp>

#include <toucan/support/spinlock.hpp>
#include <toucan/support/assert.hpp>

#include <thread>
#include <type_traits>

#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace toucan {

class Scheduler;

struct Worker {
    Scheduler* scheduler = nullptr;
    Fiber* fiber = nullptr;
    ExecutionContext context;
    std::thread thread;
    SpinLock* lock = nullptr;
    size_t id;

    void Unlock() {
        ASSERT(lock != nullptr, "Should unlock only locked spinlock");
        lock->Unlock();
        lock = nullptr;
    }
};

class Scheduler {
  private:
    Scheduler(std::shared_ptr<algo::IAlgorithm> algo, size_t workers_count);

  public:
    template <typename Algo>
    static inline Scheduler Create(size_t workers_count = std::thread::hardware_concurrency()) {
        static_assert(std::is_base_of_v<algo::IAlgorithm, Algo>, "toucan::Scheduler::Create<Algo>: template Algo must be derived from toucan::algo::IAlgorithm");
        return Scheduler(std::make_shared<Algo>(), workers_count);
    }

    // User should call WaitAll before destructore
    // otherwise some of tasks may be uncompleted
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    void Spawn(FiberRoutine routine);
    void WaitAll();

    void Yield();
    void Terminate();

    void Suspend(SpinLock& sl);
    void WakeUp(Fiber* fiber);

    // User should call WaitAll before shutdown
    // otherwise some of tasks may be uncompleted
    void Shutdown();

    size_t ThreadCount() {
        return workers_.size();
    }

  private:
    void Execute(Fiber* fiber);
    void Reschedule(Fiber* fiber);
    void Destroy(Fiber* fiber);

    void SuspendWorker();
    void NotifyWorkers();

    void SwitchTo(Fiber* fiber);
    void SwitchToScheduler();

    void SpawnWorker();

    void WorkerMain(Worker* me);
    void WorkerSetup(Worker* me);
    void WorkerLoop();
    void WorkerFinalize();

  private:
    std::vector<Worker> workers_;

    twist::atomic<uint64_t> tasks_{0};
    twist::mutex wait_mutex_;
    twist::condition_variable wait_cv_;

    bool has_new_fibers_ = false;
    twist::mutex workers_mutex_;
    twist::condition_variable workers_cv_;

    twist::atomic<bool> shutdown_{false};

    std::shared_ptr<algo::IAlgorithm> algo_;
};

Worker* GetCurrentWorker();
Fiber* GetCurrentFiber();
Scheduler* GetCurrentScheduler();

}  // namespace toucan

