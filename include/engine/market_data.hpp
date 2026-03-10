#pragma once
#include <cstdint>

namespace llp::engine {

struct MarketDataEvent {
    uint32_t symbol{0};
    int64_t  best_bid_px{0};
    int32_t  best_bid_qty{0};
    int64_t  best_ask_px{0};
    int32_t  best_ask_qty{0};
    uint64_t ts_ns{0};
};

}