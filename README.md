# Celeritas: Low-Latency Exchange & Matching Engine Pipeline

C++20 · Lock-Free SPSC · Price-Time Priority Matching · Execution Reports · Market Data Publication · Nanosecond Latency Benchmarking

## What Is This?

Celeritas is a systems-level C++ project that builds the core components of a modern electronic trading venue from scratch.

It covers the full order lifecycle (inbound order flow, price-time priority matching, execution report generation, and top-of-book market data publication), connected by lock-free SPSC queues and measured with nanosecond-resolution latency benchmarking.

This is not a tutorial project. Every design decision (lock-free queues, cache-aligned structs, explicit warmup phases, dedicated pinned threads) reflects how real low-latency trading infrastructure is actually built.

## Architecture

```
Order Generator (Producer Thread)
         │
         ▼
 [Inbound SPSC Queue]
         │
         ▼
 Matching Engine Thread
         │
         ├── Price-time priority order book
         ├── Order lifecycle management
         ├── Execution report generation
         └── Top-of-book market data updates
         │
         ├──► [Exec Report SPSC Queue] ──► Exec Publisher Thread
         └──► [Market Data SPSC Queue] ──► MD Publisher Thread
```

## Key Design Decisions

| Decision | Reason |
|----------|--------|
| SPSC lock-free ring buffers | Zero mutex overhead in the hot path |
| Cache-aligned message structs | Eliminates false sharing between threads |
| No dynamic allocation in hot path | Avoids heap latency spikes |
| Dedicated publisher threads | Decouples matching from downstream consumers |
| Explicit warmup phase | Excludes cold-start noise from measurements |
| Deterministic checksum validation | Confirms reproducible engine state across runs |

## Benchmark Results

Benchmarked on WSL2 (Ubuntu, GCC 13.3.0), Razer Blade 16, Windows 11.

Latency measures engine processing time per order: matching logic, exec report publication, and market data publication.

```
pipeline_runner starting...
warmup=100000
events_target=1000000
timing_mode=steady_clock_ns
processed=1000000
exec_reports=1548000
md_updates=896009
latency_ns: p50=14599  p95=50538  p99=70876  max=11464183
```

On these numbers: p50 ~14µs reflects the cost of matching, exec report publication, and market data publication through std::map-based order book internals on WSL2. The max spike (~11ms) is an OS scheduler interruption, not engine logic. Bare metal Linux with pool allocation, intrusive data structures, and SCHED_FIFO (Phase 5 roadmap) expected to yield sub-microsecond p50.

## Features

### Matching Engine
- Price-time priority (FIFO within each price level)
- Separate bid and ask books
- Full and partial fill handling
- Resting order insertion
- Cancel and reject flows

### Execution Reports

| Type | Trigger |
|------|---------|
| Ack | Order accepted |
| Fill | Order fully or partially matched |
| CancelAck | Cancel request accepted |
| Reject | Cancel for unknown order |

### Market Data
Top-of-book updates published on every best bid or best ask change. Fields: instrument ID, best bid/ask price and quantity, engine timestamp.

### Lock-Free Messaging
- Cache-aligned SPSC ring buffers
- Power-of-two capacity
- No mutexes
- Correct memory ordering semantics throughout

### Latency Measurement
- Warmup phase excludes cold-start noise
- Precise percentile reporting: p50, p95, p99, max
- Optional `--tsc` flag for CPU cycle-based measurement

## Order Lifecycle

### New Order
1. Producer generates OrderRequest into inbound queue
2. Matching engine consumes and emits Ack
3. Engine crosses against opposite book
4. Fill reports emitted for matched quantity
5. Residual quantity rests at its price level
6. Top-of-book update published if best price changes

### Cancel Order
- Producer generates CancelRequest
- If found → remove from book, emit CancelAck, publish market data update
- If not found → emit Reject

### Price-Time Priority Example
Two bids resting at price 100, qty 5 each. An ask enters at 100 x 5:
- Order 1 (older) fills completely
- Order 2 remains resting

Validated by `test_price_time_priority`.

## Build

### Linux / WSL2 (Recommended)
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# Standard run
./build/apps/pipeline_runner/pipeline_runner --warmup 100000 --events 1000000

# TSC cycle timing
./build/apps/pipeline_runner/pipeline_runner --warmup 100000 --events 1000000 --tsc
```

### Windows (MSVC)
```
cmake -S . -B build
cmake --build build --config Release
.\build\apps\pipeline_runner\Release\pipeline_runner.exe --warmup 100000 --events 1000000
```

## Tests

```
ctest --test-dir build --output-on-failure
```

| Test | Validates |
|------|-----------|
| celeritas_tests (sanity) | Monotonic timestamp source and build/link sanity |
| test_orderbook | Core order book crossing, resting, partial fills, and book state |
| test_spsc_smoke | Lock-free SPSC queue correctness under concurrent load |
| test_orderbook_l2 | L2 order book state correctness via checksum validation |
| test_proto | Binary protocol decode correctness and invalid-magic rejection |
| test_proto_sizes | Message struct size validation |
| test_matching_basic | Basic crossing and fill behaviour |
| test_matching_partial | Partial fill handling |
| test_matching_cancel | Cancel acknowledgement flow |
| test_price_time_priority | FIFO ordering at identical price levels and price priority |

10/10 tests passing.

## CI/CD

GitHub Actions runs on every push:
- Windows (MSVC) and Linux (GCC) builds
- Full ctest suite
- Validates: build correctness, matching correctness, queue correctness, protocol correctness, deterministic behaviour

## Project Phases

| Phase | Deliverable |
|-------|-------------|
| 1 | Order book data structures, basic exchange CLI |
| 2 | Lock-free SPSC pipeline, latency benchmarking, GitHub Actions CI |
| 3 | Binary protocol decoder, protocol correctness tests |
| 4 | Full matching engine, execution reports, market data publication, price-time priority tests |

## Roadmap: Phase 5
- Intrusive order book (no heap allocation per order)
- Pool allocation / fixed-size object arenas
- Zero dynamic allocation in hot path
- Binary order protocol replay
- SCHED_FIFO + CPU pinning on bare metal Linux
- TSC cycle measurement replacing wall clock
- Per-trial CSV benchmark output
- Cache-line and false-sharing analysis with perf + flamegraphs

## Why This Project Exists

Celeritas is built to understand the systems and engine design principles used in real electronic trading environments:
- Mechanical sympathy and cache-aware design
- Lock-free concurrency correctness
- Deterministic matching behaviour
- Low-latency inter-thread communication patterns
- Market data publication
- Latency observability under load

The goal is not to replicate a full production venue. It is to build a credible, performance-aware, exchange-style core in a clean, testable, and reviewable way, and to understand exactly why each design decision was made.

## Stack

C++20 · CMake · GCC / Clang / MSVC · Lock-Free SPSC · GitHub Actions

## Author

Jesse, Bachelor of Software Engineering (Honours), Macquarie University
[github.com/jesseosu](https://github.com/jesseosu)
Building low-latency systems and trading infrastructure.
