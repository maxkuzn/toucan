#pragma once

#include "gtest/gtest.h"

#include <toucan/algo/algorithm.hpp>

#include <toucan/algo/fifo.hpp>
#include <toucan/algo/work_stealing.hpp>

namespace toucan {
namespace testing {

using AllAlgorithmsTypes = ::testing::Types<
    algo::FIFO,
    algo::WorkStealing
>;

#define TEST_CASE_WITH_ALL_ALGOS(TestCaseName)          \
template <typename Algo>                                \
class TestCaseName : public ::testing::Test {          \
  protected:                                            \
    std::shared_ptr<algo::IAlgorithm> algo = std::make_shared<Algo>(); \
};                                                      \
                                                        \
TYPED_TEST_SUITE(TestCaseName, AllAlgorithmsTypes)

}  // namespace testing
}  // namespace toucan

