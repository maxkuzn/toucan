#pragma once

#include <toucan/core/fiber.hpp>

namespace toucan {

void Yield();

void Terminate();

void Spawn(FiberRoutine routine);

}  // namespace toucan

