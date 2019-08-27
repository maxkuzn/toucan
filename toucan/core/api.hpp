#pragma once

#include <functional>

namespace toucan {

using FiberRoutine = std::function<void()>;

void Yield();

void Terminate();

void Spawn(FiberRoutine routine);

}  // namespace toucan

