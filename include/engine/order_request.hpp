#pragma once
#include <cstdint>

namespace llp::engine {

enum class RequestType : uint8_t {
    NewOrder  = 0,
    Cancel    = 1,
    End       = 255
};

enum class Side : uint8_t {
    Buy  = 0,
    Sell = 1
};

struct OrderRequest {
    RequestType type{RequestType::NewOrder};
    Side        side{Side::Buy};
    uint64_t    order_id{0};
    int64_t     price{0};
    int32_t     qty{0};
    uint32_t    symbol{0};
    uint64_t    t0_ns{0};   // producer timestamp for latency measurement
};

}