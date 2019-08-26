#pragma once

#include <toucan/core/api.hpp>
#include <toucan/core/fiber.hpp>

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
    Scheduler(size_t workers_count = std::thread::hardware_concurrency());
    virtual ~Scheduler();

    void Spawn(FiberRoutine routine);
    void Yield();
    void Terminate();

    void Shutdown();

  protected:  // some of them private ?
    virtual Fiber* PickNextFiber() = 0;
    virtual void Schedule(Fiber* fiber) = 0;
    virtual void Reschedule(Fiber* fiber) = 0;

    void Execute(Fiber* fiber);
    void Destroy(Fiber* fiber);

    const std::vector<Worker>& GetWorkers() const {
        return workers_;
    }

  private:
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
};

Fiber* GetCurrentFiber();
Scheduler* GetCurrentScheduler();

}  // namespace toucan

