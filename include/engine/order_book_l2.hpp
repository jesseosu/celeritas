#pragma once
#include <array>
#include <cstdint>
#include "feed/md_event.hpp"

namespace llp::engine {

constexpr uint32_t MAX_INSTR = 1024;

struct TopOfBook {
  int64_t bid_px{0}; int32_t bid_qty{0};
  int64_t ask_px{0}; int32_t ask_qty{0};
};

class OrderBookL2 {
public:
  void apply(const llp::feed::MdEvent& e) noexcept {
    if (e.instrument_id >= MAX_INSTR) return;
    auto& tob = book_[e.instrument_id];

    using llp::feed::EventType;
    using llp::feed::Side;

    if (e.type == EventType::Add || e.type == EventType::Modify) {
      if (e.side == Side::Bid) { tob.bid_px = e.price; tob.bid_qty = e.qty; }
      else                    { tob.ask_px = e.price; tob.ask_qty = e.qty; }
    } else if (e.type == EventType::Cancel) {
      if (e.side == Side::Bid) tob.bid_qty = 0;
      else                    tob.ask_qty = 0;
    }
  }

  uint64_t checksum() const noexcept {
    // tiny deterministic checksum (FNV-ish)
    uint64_t h = 1469598103934665603ull;
    for (const auto& t : book_) {
      h ^= static_cast<uint64_t>(t.bid_px); h *= 1099511628211ull;
      h ^= static_cast<uint64_t>(t.ask_px); h *= 1099511628211ull;
      h ^= static_cast<uint64_t>(static_cast<uint32_t>(t.bid_qty)); h *= 1099511628211ull;
      h ^= static_cast<uint64_t>(static_cast<uint32_t>(t.ask_qty)); h *= 1099511628211ull;
    }
    return h;
  }

private:
  std::array<TopOfBook, MAX_INSTR> book_{};
};
}