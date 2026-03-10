#pragma once
#include <cstdint>

namespace llp::exchange {

enum class ExecType : uint8_t {
    Ack       = 0,
    Fill      = 1,
    CancelAck = 2,
    Reject    = 3,
    End       = 255
};

struct ExecReport {
    ExecType type{ExecType::Ack};
    uint64_t order_id{0};
    uint64_t matched_order_id{0};   // filled by Fill reports
    int64_t  price{0};              // fill price
    int32_t  qty{0};                // filled qty
    int32_t  leaves_qty{0};         // remaining qty after fill
    uint64_t t0_ns{0};              // original producer timestamp
    uint64_t t_eng_ns{0};           // engine processing timestamp
};

}