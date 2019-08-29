#pragma once

#include <toucan/core/api.hpp>
#include <toucan/core/fiber.hpp>

#include <toucan/algo/algorithm.hpp>

#include <toucan/support/spinlock.hpp>

#include <thread>

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
};

class Scheduler {
  public:
    Scheduler(std::shared_ptr<algo::IAlgorithm> algo, size_t workers_count = std::thread::hardware_concurrency());

    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    void Spawn(FiberRoutine routine);
    void WaitAll();

    void Yield();
    void Terminate();

    void Shutdown();

    void Suspend(SpinLock& sl);
    void WakeUp(Fiber* fiber);

  private:
    void Execute(Fiber* fiber);
    void Reschedule(Fiber* fiber);
    void Destroy(Fiber* fiber);

    const std::vector<Worker>& GetWorkers() const {
        return workers_;
    }

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

    twist::atomic<bool> started_{false};
    twist::atomic<bool> shutdown_{false};

    std::shared_ptr<algo::IAlgorithm> algo_;
};

Worker* GetCurrentWorker();
Fiber* GetCurrentFiber();
Scheduler* GetCurrentScheduler();

}  // namespace toucan

