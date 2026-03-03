#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace llp::ipc {

// Capacity must be power of two
template <typename T, size_t CapacityPow2>
class SpscRingBuffer {
  static_assert((CapacityPow2 & (CapacityPow2 - 1)) == 0, "Capacity must be power of two");
  static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

public:
  bool push(const T& v) noexcept {
    const auto head = head_.load(std::memory_order_relaxed);
    const auto next = (head + 1) & mask_;
    if (next == tail_.load(std::memory_order_acquire)) return false; // full

    buffer_[head] = v;
    head_.store(next, std::memory_order_release);
    return true;
  }

  bool pop(T& out) noexcept {
    const auto tail = tail_.load(std::memory_order_relaxed);
    if (tail == head_.load(std::memory_order_acquire)) return false; // empty

    out = buffer_[tail];
    tail_.store((tail + 1) & mask_, std::memory_order_release);
    return true;
  }

private:
  static constexpr size_t mask_ = CapacityPow2 - 1;
  alignas(64) std::atomic<size_t> head_{0};
  alignas(64) std::atomic<size_t> tail_{0};
  alignas(64) T buffer_[CapacityPow2]{};
};

}
