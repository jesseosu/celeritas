#pragma once
#include <cstdint>

namespace celeritas {

// Monotonic timestamp in nanoseconds (steady clock)
std::uint64_t now_ns();

}