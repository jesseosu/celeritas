#include "engine/execution_engine.hpp"

#include "common/timestamp.hpp"

namespace llp::engine {

ExecutionEngine::ExecutionEngine(Queue& queue, llp::metrics::LatencyTracker& latency)
    : queue_(queue), latency_(latency) {}

EngineStats ExecutionEngine::run() {
  using llp::feed::EventType;
  using llp::feed::MdEvent;

  EngineStats stats{};
  MdEvent e{};

  while (true) {
    if (!queue_.pop(e)) {
      continue; // spin
    }

    if (e.type == EventType::End) {
      stats.end_seen = true;
      break;
    }

    const uint64_t t1_ns = llp::time::now_ns();
    if (e.t0_ns != 0) latency_.record(t1_ns - e.t0_ns);

    stats.processed++;
  }

  return stats;
}

}
