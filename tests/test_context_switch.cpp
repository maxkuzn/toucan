#include "gtest/gtest.h"

#include <toucan/core/context.hpp>
#include <toucan/core/stack.hpp>

using namespace toucan;

ExecutionContext main_ctx;
ExecutionContext secondary_ctx;

bool flag = false;
void foo() {
    flag = true;
    secondary_ctx.SwitchTo(main_ctx);
}

TEST(ContextSwitch, DoubleSwitch) {
    auto stack = FiberStack::Allocate();
    secondary_ctx.Setup(stack, foo);
    ASSERT_FALSE(flag);
    main_ctx.SwitchTo(secondary_ctx);
    ASSERT_TRUE(flag);
}

