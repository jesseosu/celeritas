#pragma once
#include <cstdint>
#include <vector>

namespace llp::metrics {

class LatencyTracker {
public:
  void reserve(size_t n);
  void record(uint64_t delta_ns);
  struct Summary {
    uint64_t count{0};
    uint64_t p50{0}, p95{0}, p99{0}, max{0};
  };
  Summary summarize();

private:
  std::vector<uint64_t> samples_;
};

}
