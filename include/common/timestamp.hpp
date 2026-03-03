#pragma once
#include <cstdint>
#include <chrono>

namespace llp::time {

inline uint64_t now_ns() {
  using namespace std::chrono;
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

}
