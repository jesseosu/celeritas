// apps/pipeline_runner/main.cpp
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <thread>

#include "common/affinity.hpp"
#include "common/timestamp.hpp"
#include "common/tsc.hpp"
#include "engine/matching_engine.hpp"
#include "engine/order_request.hpp"
#include "engine/market_data.hpp"
#include "exchange/exec_report.hpp"
#include "ipc/spsc_ring_buffer.hpp"
#include "metrics/latency.hpp"

static uint64_t parse_u64_arg(int argc, char** argv, const char* flag, uint64_t def) {
  for (int i = 1; i + 1 < argc; i++) {
    if (std::string_view(argv[i]) == flag)
      return static_cast<uint64_t>(std::strtoull(argv[i + 1], nullptr, 10));
  }
  return def;
}

static bool has_flag(int argc, char** argv, const char* flag) {
  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == flag) return true;
  return false;
}

int main(int argc, char** argv) {
  using namespace llp::engine;
  using llp::exchange::ExecReport;
  using llp::exchange::ExecType;

  const uint64_t warmup   = parse_u64_arg(argc, argv, "--warmup", 100'000);
  const uint64_t events   = parse_u64_arg(argc, argv, "--events", 1'000'000);
  const bool     use_tsc  = has_flag(argc, argv, "--tsc");

  constexpr uint32_t PRODUCER_CORE = 0;
  constexpr uint32_t ENGINE_CORE   = 1;
  constexpr uint32_t EXEC_PUB_CORE = 2;
  constexpr uint32_t MD_PUB_CORE   = 3;

  std::cout << "pipeline_runner starting...\n"
            << "warmup="        << warmup << "\n"
            << "events_target=" << events << "\n"
            << "timing_mode="   << (use_tsc ? "tsc_cycles" : "steady_clock_ns") << "\n"
            << std::flush;

  // Queues
  static InQueue   in_q;
  static ExecQueue exec_q;
  static MdQueue   md_q;

  // Latency tracker
  llp::metrics::LatencyTracker lat;
  lat.reserve(static_cast<size_t>(events));

  // Shared stats
  std::atomic<uint64_t> exec_reports{0};
  std::atomic<uint64_t> md_updates{0};
  std::atomic<bool>     started{false};
  std::atomic<bool>     warmup_done{false};

  // ── Exec Publisher Thread ──────────────────────────────────────────────
  std::thread exec_publisher([&] {
    llp::sys::pin_thread_to_cpu(EXEC_PUB_CORE);
    llp::sys::set_high_priority();

    ExecReport r{};
    while (true) {
      if (!exec_q.pop(r)) { llp::time::cpu_relax(); continue; }
      if (r.type == ExecType::End) break;
      exec_reports.fetch_add(1, std::memory_order_relaxed);
    }
  });

  // ── MD Publisher Thread ────────────────────────────────────────────────
  std::thread md_publisher([&] {
    llp::sys::pin_thread_to_cpu(MD_PUB_CORE);
    llp::sys::set_high_priority();

    MarketDataEvent md{};
    // MD publisher runs until exec publisher signals done
    // We use a sentinel md event with symbol == UINT32_MAX
    while (true) {
      if (!md_q.pop(md)) { llp::time::cpu_relax(); continue; }
      if (md.symbol == UINT32_MAX) break;
      md_updates.fetch_add(1, std::memory_order_relaxed);
    }
  });

  // ── Matching Engine Thread ─────────────────────────────────────────────
  std::thread engine_thread([&] {
    llp::sys::pin_thread_to_cpu(ENGINE_CORE);
    llp::sys::set_high_priority();

    started.store(true, std::memory_order_release);

    MatchingEngine engine(in_q, exec_q, md_q, lat);
    auto stats = engine.run();

    (void)stats;

    // Signal exec publisher to stop
    ExecReport end{};
    end.type = ExecType::End;
    while (!exec_q.push(end)) llp::time::cpu_relax();

    // Signal md publisher to stop
    MarketDataEvent md_end{};
    md_end.symbol = UINT32_MAX;
    while (!md_q.push(md_end)) llp::time::cpu_relax();
  });

  // ── Producer Thread ────────────────────────────────────────────────────
  std::thread producer([&] {
    llp::sys::pin_thread_to_cpu(PRODUCER_CORE);
    llp::sys::set_high_priority();

    while (!started.load(std::memory_order_acquire))
      llp::time::cpu_relax();

    uint64_t next_id = 1;

    // Warmup phase — no latency measurement
    for (uint64_t i = 0; i < warmup; i++) {
      OrderRequest req{};
      req.type     = RequestType::NewOrder;
      req.side     = (i & 1) ? Side::Sell : Side::Buy;
      req.order_id = next_id++;
      req.price    = 100000 + static_cast<int64_t>(i % 50) * 100;
      req.qty      = 10;
      req.symbol   = 1;
      req.t0_ns    = 0; // not measured

      while (!in_q.push(req)) llp::time::cpu_relax();
    }

    warmup_done.store(true, std::memory_order_release);

    // Measured phase
    // Mix of new orders and cancels to exercise full lifecycle
    // Every 10th event is a cancel of a recent order
    uint64_t last_id = next_id - 1;

    for (uint64_t i = 0; i < events; i++) {
      OrderRequest req{};

      if (i % 10 == 9 && last_id >= 1) {
        // Cancel a recent resting order
        req.type     = RequestType::Cancel;
        req.order_id = last_id - (last_id % 2); // cancel an even (buy) order
        req.symbol   = 1;
        req.t0_ns    = use_tsc ? llp::time::rdtsc() : llp::time::now_ns();
      } else {
        req.type     = RequestType::NewOrder;
        req.side     = (i & 1) ? Side::Sell : Side::Buy;
        req.order_id = next_id++;
        req.price    = 100000 + static_cast<int64_t>(i % 50) * 100;
        req.qty      = 10;
        req.symbol   = 1;
        req.t0_ns    = use_tsc ? llp::time::rdtsc() : llp::time::now_ns();
        last_id      = req.order_id;
      }

      while (!in_q.push(req)) llp::time::cpu_relax();
    }

    // Send End sentinel
    OrderRequest end{};
    end.type = RequestType::End;
    while (!in_q.push(end)) llp::time::cpu_relax();
  });

  producer.join();
  engine_thread.join();
  exec_publisher.join();
  md_publisher.join();

  auto s = lat.summarize();

  std::cout
    << "processed="   << events << "\n"
    << "exec_reports=" << exec_reports.load() << "\n"
    << "md_updates="  << md_updates.load() << "\n"
    << (use_tsc ? "latency_cycles: p50=" : "latency_ns: p50=") << s.p50
    << " p95=" << s.p95
    << " p99=" << s.p99
    << " max=" << s.max << "\n";

  return 0;
}