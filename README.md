# Toucan
Toucan is a C++ library with multi-threaded task scheduler

# Installation

```
# Checkout library
$ git clone https://github.com/maxkuzn/toucan.git
# Go to root directory
$ cd toucan
# Create directory for build
$ mkdir build && cd build
# Generate Makefiles
$ cmake ../
# Build library
$ make
```

If you want, you can pass `-DTOUCAN_TEST=ON` to cmake to enable tests for toucan (they will be lay in build/tests/bin/) and `-DTOUCAN_BENCHMARK=ON` to enable benchmarks (they will be in build/benchmark/bin/)

# Add to existing CMake project

You should download Toucan source code. You can do it in different ways, such as download mannualy, checkout as git submodule or use CMake to download TOucan as part of the build's configure step.

After you did this, you have to add next commands to your CMakeLists.txt
```
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/toucan)

# Now simply link against toucan
add_executable(example example.cpp)
target_link_libraries(example toucan)
```

# Usage

There is an example of usage 

```
#include <toucan/core/scheduler.hpp>
#include <toucan/core/api.hpp>
#include <toucan/algo/global_fifo.hpp>
#include <toucan/sync/mutex.hpp>

void foo() {
  // Does something
}

int main() {
  # Template argument specify algorithm that will be used by scheduler
  # (See more algorithms in toucan/toucan/algo/)
  # Argument specify number of system threads used by scheduler
  auto scheduler = toucan::Scheduler::Create<toucan::algo::GlobalFIFO>(4);
  scheduler.Spawn(foo);

  toucan::Mutex mutex;
  int counter = 0;
  for (size_t i = 0; i < 42; i++) {
    scheduler.Spawn([&] {
      mutex.Lock();
      counter++;
      Yield();
      mutex.Unlock();
    }
  }
  // Here counter can be from 0 to 42
  scheduler.WaitAll();
  // Here counter will be 42
}
```

Notice that you have to use `touca::Yield()` to allow scheduler reschedule current task, oservise there can be small fairness.
Also notice that you have to use `toucan::Mutex` and `toucan::ConditionVariable` insted of `std::mutex` and `std::condition_variable` because they do not block current thread and allow to execute another task, while first task is waiting.
