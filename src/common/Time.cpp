#include "celeritas/common/Time.hpp"
#include <chrono>

namespace celeritas {

  std::uint64_t now_ns() {
    using namespace std::chrono;
    const auto t = steady_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(duration_cast<nanoseconds>(t).count());
  }

} 