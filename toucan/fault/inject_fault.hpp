#pragma once

#include <toucan/fault/adversary.hpp>

namespace toucan {
namespace fault {

inline void InjectFault() {
  GetAdversary()->Fault();
}

}  // namespace fault
}  // namespace toucan

// Voluntarily inject fault (yield/sleep/park) into current thread

#if defined(TOUCAN_FAULTY)

#define TOUCAN_VOLUNTARY_FAULT() toucan::fault::InjectFault();

#else

#define TOUCAN_VOLUNTARY_FAULT()

#endif

