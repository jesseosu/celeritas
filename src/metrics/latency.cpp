#include "metrics/latency.hpp"
#include <algorithm>

namespace llp::metrics {

void LatencyTracker::reserve(size_t n) { samples_.reserve(n); }
void LatencyTracker::record(uint64_t d) { samples_.push_back(d); }

LatencyTracker::Summary LatencyTracker::summarize() {
  Summary s;
  s.count = samples_.size();
  if (samples_.empty()) return s;

  std::sort(samples_.begin(), samples_.end());
  auto pct = [&](double p) -> uint64_t {
    size_t idx = static_cast<size_t>(p * (samples_.size() - 1));
    return samples_[idx];
  };

  s.p50 = pct(0.50);
  s.p95 = pct(0.95);
  s.p99 = pct(0.99);
  s.max = samples_.back();
  return s;
}

}
