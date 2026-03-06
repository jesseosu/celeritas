#pragma once
#include <array>
#include <cstdint>
#include <algorithm>

namespace llp::metrics {

// Histogram of uint64 values using log2 buckets.
// Buckets cover [0..2^63]. Good enough for latency ns/cycles.
class Log2Histogram {
public:
  static constexpr size_t B = 64;

  void clear() noexcept { counts_.fill(0); total_ = 0; max_ = 0; }

  void record(uint64_t v) noexcept {
    const uint8_t b = bucket(v);
    counts_[b]++;
    total_++;
    if (v > max_) max_ = v;
  }

  uint64_t total() const noexcept { return total_; }
  uint64_t max() const noexcept { return max_; }

  // Approx percentile: returns upper bound of bucket.
  uint64_t percentile(double p) const noexcept {
    if (total_ == 0) return 0;
    const uint64_t target = (uint64_t)(p * (double)(total_ - 1));
    uint64_t acc = 0;
    for (size_t i = 0; i < B; i++) {
      uint64_t c = counts_[i];
      if (acc + c > target) {
        // upper bound of bucket i
        return (i == 0) ? 0 : (1ull << i);
      }
      acc += c;
    }
    return max_;
  }

private:
  static uint8_t bucket(uint64_t v) noexcept {
    if (v == 0) return 0;
    uint8_t b = 0;
    while (v >>= 1) b++;
    return b;
  }

  std::array<uint64_t, B> counts_{};
  uint64_t total_{0};
  uint64_t max_{0};
};

}