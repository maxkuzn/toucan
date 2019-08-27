#pragma once

#include <toucan/core/api.hpp>
#include <toucan/core/fiber.hpp>

#include <toucan/algo/algorithm.hpp>

#include <thread>
#include <condition_variable>
#include <atomic>
#include <mutex>

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
    Scheduler(std::shared_ptr<algo::Algorithm> algo, size_t workers_count = std::thread::hardware_concurrency());

    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    void Spawn(FiberRoutine routine);
    void Yield();
    void Terminate();

    void Shutdown();

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

    std::atomic<bool> started_{false};
    std::atomic<bool> shutdown_{false};

    std::shared_ptr<algo::Algorithm> algo_;
};

Fiber* GetCurrentFiber();
Scheduler* GetCurrentScheduler();

}  // namespace toucan

