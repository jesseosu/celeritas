# Celeritas — Low-Latency Execution Pipeline

> A high-performance, lock-free, multi-threaded execution pipeline prototype inspired by modern electronic trading infrastructure.

Celeritas is a C++ systems-level project focused on low-latency message processing, deterministic state updates, and lock-free inter-thread communication.

This project simulates the core architecture of a trading engine:

- Market data ingest  
- Lock-free message passing  
- Deterministic state updates  
- Execution report generation  
- End-to-end latency measurement  

---

## Architecture Overview


Producer (Feed)
│
▼
[SPSC Ring Buffer]
│
▼
Execution Engine Thread
│
├── OrderBookL2 state update
├── Latency measurement (p50/p95/p99)
└── Ack generation
│
▼
[Output SPSC Queue]
│
▼
Publisher Thread


### Key Design Choices

- **Single Producer / Single Consumer lock-free ring buffers**
- No dynamic allocation in the hot path
- Fixed-size message structs
- Deterministic synthetic feed generation
- Percentile latency reporting
- Cross-platform build (Windows + Linux)

---

## Features

### Lock-Free Messaging
- Cache-aligned SPSC ring buffer
- Power-of-two capacity
- No mutexes
- Correct memory ordering semantics

### Execution Engine
- Continuous spin-based consumer loop
- Deterministic shutdown via sentinel message
- OrderBook L2 baseline implementation
- Execution acknowledgements via output queue

### Latency Measurement
- Nanosecond timestamps
- End-to-end measurement (producer → engine)
- Percentile summary:
  - p50
  - p95
  - p99
  - max

### Deterministic State Validation
- OrderBookL2 checksum output
- Execution report count validation
- Processed message count verification

---

## Build Instructions

### Windows (MSVC)

```bash
cmake -S . -B build
cmake --build build --config Release

Run:

.\build\apps\pipeline_runner\Release\pipeline_runner.exe --events 1000000
Linux / WSL
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

Run:

./build/apps/pipeline_runner/pipeline_runner --events 1000000
Example Output
pipeline_runner starting...
events_target=1000000
processed=1000000
reports=1000000
book_checksum=8602266223694710646
latency_ns: p50=15451500 p95=20369500 p99=24187300 max=25224200


Validation guarantees:

processed == events_target
reports == processed
book_checksum != 0
exit code = 0

Testing


Run the full test suite:
ctest --test-dir build -C Release --output-on-failure


Includes:

SPSC ring buffer stress test
OrderBook L2 correctness test
Protocol decoding tests
Message size validation
Continuous Integration
GitHub Actions runs:
Windows build
Linux build
Full test suite via ctest


Every push validates:

Build correctness
Threaded correctness
Deterministic behavior
Current Phase Status


Phase 2 Complete

Threaded producer → engine → publisher pipeline
Lock-free SPSC messaging
Deterministic order book state updates
Execution acknowledgements
Percentile latency reporting
Unit tests + CI


Roadmap (Phase 3)

Cache line padding & false sharing elimination
CPU pinning & thread affinity
TSC-based cycle measurement
Heap elimination in hot path
Branch prediction tuning
Binary protocol decoding in hot loop
Microbenchmark harness
Flamegraph / perf analysis


Why This Project Exists

Celeritas is not a toy matching engine.
It is a deliberate exploration of:
Mechanical sympathy
Memory layout
Lock-free design
Deterministic concurrency
Latency observability


The goal is to build systems the way high-performance trading infrastructure is built — incrementally, measurably, and correctly.


Tech Stack

C++20
CMake
MSVC / GCC / Clang
Lock-free ring buffer
GitHub Actions CI