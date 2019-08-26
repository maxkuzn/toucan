#pragma once

#include <toucan/core/stack.hpp>

namespace toucan {

typedef void (*Routine)();

struct ExecutionContext {
    void* rsp = nullptr;

    void Setup(const FiberStack& stack, Routine routine);
    void SwitchTo(ExecutionContext& target);
};

}  // namespace toucan

