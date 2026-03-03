// apps/pipeline_runner/main.cpp
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "common/timestamp.hpp"
#include "engine/order_book_l2.hpp"
#include "exchange/exec_report.hpp"
#include "feed/md_event.hpp"
#include "ipc/spsc_ring_buffer.hpp"
#include "metrics/latency.hpp"

// Tiny CLI helper (optional): --events N
static uint64_t parse_u64_arg(int argc, char** argv, const char* flag, uint64_t def) {
  for (int i = 1; i + 1 < argc; i++) {
    if (std::string_view(argv[i]) == flag) {
      return static_cast<uint64_t>(std::strtoull(argv[i + 1], nullptr, 10));
    }
  }
  return def;
}

int main(int argc, char** argv) {
  using llp::feed::MdEvent;
  using llp::feed::EventType;
  using llp::feed::Side;

  using llp::exchange::ExecReport;
  using llp::exchange::ExecType;

  // Keep big buffers off the stack (Windows stack overflow otherwise)
  constexpr size_t IN_CAP  = 1u << 16;  // 65536
  constexpr size_t OUT_CAP = 1u << 16;  // 65536

  static llp::ipc::SpscRingBuffer<MdEvent, IN_CAP> in_q;
  static llp::ipc::SpscRingBuffer<ExecReport, OUT_CAP> out_q;

  const uint64_t events = parse_u64_arg(argc, argv, "--events", 1'000'000);

  std::cout << "pipeline_runner starting...\n";
  std::cout << "events_target=" << events << "\n" << std::flush;

  llp::metrics::LatencyTracker lat;
  lat.reserve(static_cast<size_t>(events));

  llp::engine::OrderBookL2 book;

  std::atomic<bool> started{false};
  std::atomic<uint64_t> processed{0};
  std::atomic<uint64_t> reports{0};

  // Publisher thread: consumes exec reports
  std::thread publisher([&] {
    ExecReport r{};
    while (true) {
      if (!out_q.pop(r)) {
        continue; // spin (baseline)
      }
      if (r.type == ExecType::End) {
        break;
      }
      reports.fetch_add(1, std::memory_order_relaxed);
    }
  });

  // Consumer (engine): consumes market data, updates state, emits acks
  std::thread consumer([&] {
    MdEvent e{};
    started.store(true, std::memory_order_release);

    while (true) {
      if (!in_q.pop(e)) {
        continue; // spin (baseline)
      }

      if (e.type == EventType::End) {
        break;
      }

      // Step 4: apply to order book/state
      book.apply(e);

      // End-to-end latency (producer -> consumer)
      const uint64_t t1_ns = llp::time::now_ns();
      if (e.t0_ns != 0) {
        lat.record(t1_ns - e.t0_ns);
      }

      // Step 5: emit minimal execution report (Ack)
      ExecReport rep{};
      rep.type = ExecType::Ack;
      rep.seq = e.seq;
      rep.t0_ns = e.t0_ns;
      rep.t_eng_ns = t1_ns;

      while (!out_q.push(rep)) {
        // spin if output queue is full
      }

      processed.fetch_add(1, std::memory_order_relaxed);
    }

    // Tell publisher to stop
    ExecReport end{};
    end.type = ExecType::End;
    while (!out_q.push(end)) {}
  });

  // Producer (feed): generates synthetic events
  std::thread producer([&] {
    while (!started.load(std::memory_order_acquire)) {}

    for (uint64_t i = 0; i < events; i++) {
      MdEvent e{};
      e.type = EventType::Add;
      e.side = (i & 1) ? Side::Ask : Side::Bid;
      e.seq = i;
      e.instrument_id = 1;
      e.price = 100000 + static_cast<int64_t>(i % 100);
      e.qty = 1;
      e.t0_ns = llp::time::now_ns();

      while (!in_q.push(e)) {
        // spin if input queue is full
      }
    }

    MdEvent end{};
    end.type = EventType::End;
    while (!in_q.push(end)) {}
  });

  producer.join();
  consumer.join();
  publisher.join();

  auto s = lat.summarize();

  std::cout
      << "processed=" << processed.load() << "\n"
      << "reports=" << reports.load() << "\n"
      << "book_checksum=" << book.checksum() << "\n"
      << "latency_ns: p50=" << s.p50
      << " p95=" << s.p95
      << " p99=" << s.p99
      << " max=" << s.max << "\n";

  return 0;
}