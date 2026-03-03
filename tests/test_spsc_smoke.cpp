#include <cassert>
#include <atomic>
#include <thread>
#include "ipc/spsc_ring_buffer.hpp"

struct Msg { uint64_t x; };

int main() {
  constexpr size_t CAP = 1 << 12;
  static llp::ipc::SpscRingBuffer<Msg, CAP> q;

  constexpr uint64_t N = 1'000'00;
  std::atomic<uint64_t> got{0};

  std::thread c([&]{
    Msg m{};
    while (got.load() < N) {
      if (q.pop(m)) got.fetch_add(1);
    }
  });

  std::thread p([&]{
    for (uint64_t i=0;i<N;i++) {
      Msg m{i};
      while (!q.push(m)) {}
    }
  });

  p.join(); c.join();
  assert(got.load() == N);
  return 0;
}