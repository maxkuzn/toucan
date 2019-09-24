#pragma once

#include "gtest/gtest.h"

#include <toucan/algo/algorithm.hpp>

#include <toucan/algo/global_fifo.hpp>
#include <toucan/algo/work_stealing.hpp>
#include <toucan/algo/work_stealing_support/mutex_queue.hpp>
#include <toucan/algo/work_stealing_support/lf_ab_queue.hpp>

namespace toucan {
namespace testing {

using AllAlgorithmsTypes = ::testing::Types<
    algo::GlobalFIFO,
    algo::WorkStealing<algo::MutexQueue, algo::MutexQueue>,
    algo::WorkStealing<algo::MutexQueue, algo::LFABQueue>
>;

#define TEST_CASE_WITH_ALL_ALGOS(TestCaseName)          \
template <typename Algo>                                \
class TestCaseName : public ::testing::Test {           \
};                                                      \
                                                        \
TYPED_TEST_SUITE(TestCaseName, AllAlgorithmsTypes)

}  // namespace testing
}  // namespace toucan

