#pragma once
#include <cstdint>
#include "metrics/histogram.hpp"

namespace llp::metrics {

class LatencyTracker {
public:
  struct Summary {
    uint64_t count{0};
    uint64_t p50{0}, p95{0}, p99{0}, max{0};
  };

  void clear() noexcept { hist_.clear(); }
  void record(uint64_t delta) noexcept { hist_.record(delta); }

  Summary summarize() const noexcept {
    Summary s;
    s.count = hist_.total();
    s.p50 = hist_.percentile(0.50);
    s.p95 = hist_.percentile(0.95);
    s.p99 = hist_.percentile(0.99);
    s.max = hist_.max();
    return s;
  }

private:
  Log2Histogram hist_;
};

}