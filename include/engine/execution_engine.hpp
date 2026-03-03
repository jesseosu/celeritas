#pragma once
#include <cstdint>

#include "feed/md_event.hpp"
#include "ipc/spsc_ring_buffer.hpp"
#include "metrics/latency.hpp"

namespace llp::engine {

struct EngineStats {
  uint64_t processed{0};
  bool end_seen{false};
};

class ExecutionEngine {
public:
  // Use the exact queue type you’ve chosen
  using Queue = llp::ipc::SpscRingBuffer<llp::feed::MdEvent, (1u << 16)>;

  ExecutionEngine(Queue& queue, llp::metrics::LatencyTracker& latency);

  EngineStats run(); // blocks until End

private:
  Queue& queue_;
  llp::metrics::LatencyTracker& latency_;
};

}
