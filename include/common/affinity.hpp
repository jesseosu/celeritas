#pragma once
#include <cstdint>

#if defined(_WIN32)
    #define NOMINMAX
    #include <windows.h>
#else
    #include <pthread.h>
    #include <sched.h>
#endif

namespace llp::sys {

inline bool pin_thread_to_cpu(uint32_t cpu_index) noexcept {
#if defined(_WIN32)
    const DWORD_PTR mask = (DWORD_PTR)1 << cpu_index;
    const HANDLE h = GetCurrentThread();
    return SetThreadAffinityMask(h, mask) != 0;
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_index, &cpuset);
    eturn pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0;
#endif
}

inline bool set_high_priority() noexcept {
#if defined(_WIN32)
    return SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) != 0;
#else
  // On Linux this may require CAP_SYS_NICE for real-time priorities.
  // Keep as a best-effort no-op for now.
    return true;
#endif
}

} // namespace llp::sys