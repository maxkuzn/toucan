#include <toucan/core/api.hpp>

#include <toucan/core/scheduler.hpp>

namespace toucan {

void Yield() {
    GetCurrentScheduler()->Yield();
}

void Terminate() {
    GetCurrentScheduler()->Terminate();
}

void Spawn(FiberRoutine routine) {
    GetCurrentScheduler()->Spawn(routine);
}

}  // namespace toucan
