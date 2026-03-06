#pragma once
#include <cstdint>

#if defined(_MSC_VER)
    #include <intrin.h>
#elif defined(__x86_64__) || defined(__i386__)
    #include <x86intrin.h>
#endif

namespace llp::time {

// Serialize + read TSC (cycles). RDTSCP is partially serializing.
inline uint64_t rdtsc() noexcept {
#if defined(_MSC_VER)
    return __rdtsc();
#elif defined(__x86_64__) || defined(__i386__)
    return __rdtsc();
#else
  return 0; // non-x86: implement with chrono fallback if needed
#endif
}

inline uint64_t rdtscp() noexcept {
#if defined(_MSC_VER)
    unsigned int aux = 0;
    return __rdtscp(&aux);
#elif defined(__x86_64__) || defined(__i386__)
    unsigned int aux = 0;
    return __rdtscp(&aux);
#else
    return 0;
#endif
}

// Small pause hint for spin loops
inline void cpu_relax() noexcept {
#if defined(_MSC_VER)
    _mm_pause();
#elif defined(__x86_64__) || defined(__i386__)
    _mm_pause();
#else
  // fallback: no-op
#endif
}

} // namespace llp::time