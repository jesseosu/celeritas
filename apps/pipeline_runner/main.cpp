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
#include "engine/order_book_l2.hpp"
#include "exchange/exec_report.hpp"
#include "feed/md_event.hpp"
#include "ipc/spsc_ring_buffer.hpp"
#include "metrics/latency.hpp"

static uint64_t parse_u64_arg(int argc, char** argv, const char* flag, uint64_t def) {
  for (int i = 1; i + 1 < argc; i++) {
    if (std::string_view(argv[i]) == flag) {
      return static_cast<uint64_t>(std::strtoull(argv[i + 1], nullptr, 10));
    }
  }
  return def;
}

static bool has_flag(int argc, char** argv, const char* flag) {
  for (int i = 1; i < argc; i++) {
    if (std::string_view(argv[i]) == flag) {
      return true;
    }
  }
  return false;
}

int main(int argc, char** argv) {
  using llp::feed::MdEvent;
  using llp::feed::EventType;
  using llp::feed::Side;

  using llp::exchange::ExecReport;
  using llp::exchange::ExecType;

  constexpr size_t IN_CAP  = 1u << 16;
  constexpr size_t OUT_CAP = 1u << 16;

  static llp::ipc::SpscRingBuffer<MdEvent, IN_CAP> in_q;
  static llp::ipc::SpscRingBuffer<ExecReport, OUT_CAP> out_q;

  const uint64_t warmup = parse_u64_arg(argc, argv, "--warmup", 100'000);
  const uint64_t events = parse_u64_arg(argc, argv, "--events", 1'000'000);
  const bool use_tsc    = has_flag(argc, argv, "--tsc");

  // Simple fixed core mapping for now
  constexpr uint32_t PRODUCER_CORE  = 0;
  constexpr uint32_t CONSUMER_CORE  = 1;
  constexpr uint32_t PUBLISHER_CORE = 2;

  std::cout << "pipeline_runner starting...\n";
  std::cout << "warmup=" << warmup << "\n";
  std::cout << "events_target=" << events << "\n";
  std::cout << "timing_mode=" << (use_tsc ? "tsc_cycles" : "steady_clock_ns") << "\n"
            << std::flush;

  llp::metrics::LatencyTracker lat;
  // If your new LatencyTracker no longer has reserve(), remove this line.
  lat.reserve(static_cast<size_t>(events));

  llp::engine::OrderBookL2 book;

  std::atomic<bool> started{false};
  std::atomic<bool> warmup_done{false};

  std::atomic<uint64_t> processed{0};
  std::atomic<uint64_t> reports{0};

  // Publisher thread
  std::thread publisher([&] {
    llp::sys::pin_thread_to_cpu(PUBLISHER_CORE);
    llp::sys::set_high_priority();

    ExecReport r{};
    while (true) {
      if (!out_q.pop(r)) {
        llp::time::cpu_relax();
        continue;
      }

      if (r.type == ExecType::End) {
        break;
      }

      reports.fetch_add(1, std::memory_order_relaxed);
    }
  });

  // Consumer thread
  std::thread consumer([&] {
    llp::sys::pin_thread_to_cpu(CONSUMER_CORE);
    llp::sys::set_high_priority();

    MdEvent e{};
    started.store(true, std::memory_order_release);

    while (true) {
      if (!in_q.pop(e)) {
        llp::time::cpu_relax();
        continue;
      }

      if (e.type == EventType::End) {
        break;
      }

      // Step 4: apply event to state
      book.apply(e);

      const uint64_t t1 = use_tsc ? llp::time::rdtscp() : llp::time::now_ns();

      // Only record latency after warmup is complete
      if (warmup_done.load(std::memory_order_acquire)) {
        if (e.t0_ns != 0) {
          lat.record(t1 - e.t0_ns);
        }
      }

      // Step 5: emit execution report
      ExecReport rep{};
      rep.type = ExecType::Ack;
      rep.seq = e.seq;
      rep.t0_ns = e.t0_ns;
      rep.t_eng_ns = t1;

      while (!out_q.push(rep)) {
        llp::time::cpu_relax();
      }

      if (warmup_done.load(std::memory_order_acquire)) {
    	processed.fetch_add(1, std::memory_order_relaxed);
	}
    }

    ExecReport end{};
    end.type = ExecType::End;
    while (!out_q.push(end)) {
      llp::time::cpu_relax();
    }
  });

  // Producer thread
  std::thread producer([&] {
    llp::sys::pin_thread_to_cpu(PRODUCER_CORE);
    llp::sys::set_high_priority();

    while (!started.load(std::memory_order_acquire)) {
      llp::time::cpu_relax();
    }

    // Warmup phase
    for (uint64_t i = 0; i < warmup; i++) {
      MdEvent e{};
      e.type = EventType::Add;
      e.side = (i & 1) ? Side::Ask : Side::Bid;
      e.seq = i;
      e.instrument_id = 1;
      e.price = 100000 + static_cast<int64_t>(i % 100);
      e.qty = 1;
      e.t0_ns = 0; // do not measure warmup latency

      while (!in_q.push(e)) {
        llp::time::cpu_relax();
      }
    }

    warmup_done.store(true, std::memory_order_release);

    // Measured phase
    for (uint64_t i = 0; i < events; i++) {
      MdEvent e{};
      e.type = EventType::Add;
      e.side = (i & 1) ? Side::Ask : Side::Bid;
      e.seq = i + warmup;
      e.instrument_id = 1;
      e.price = 100000 + static_cast<int64_t>(i % 100);
      e.qty = 1;
      e.t0_ns = use_tsc ? llp::time::rdtsc() : llp::time::now_ns();

      while (!in_q.push(e)) {
        llp::time::cpu_relax();
      }
    }

    MdEvent end{};
    end.type = EventType::End;
    while (!in_q.push(end)) {
      llp::time::cpu_relax();
    }
  });

  producer.join();
  consumer.join();
  publisher.join();

  auto s = lat.summarize();

  std::cout
      << "processed=" << (reports.load() - warmup) << "\n"
      << "reports=" << reports.load() << "\n"
      << "book_checksum=" << book.checksum() << "\n"
      << (use_tsc ? "latency_cycles: p50=" : "latency_ns: p50=") << s.p50
      << " p95=" << s.p95
      << " p99=" << s.p99
      << " max=" << s.max << "\n";

  return 0;
}
