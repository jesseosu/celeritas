#pragma once
#include <cstdint>
#include <chrono>
#include <thread>
#include "common/tsc.hpp"

namespace llp::time {

inline uint64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

inline void cpu_relax_yield() noexcept {
    llp::time::cpu_relax();
}

}