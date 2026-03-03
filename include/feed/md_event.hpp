#pragma once
#include <cstdint>

namespace llp::feed {

enum class EventType : uint8_t { Add=0, Modify=1, Cancel=2, Trade=3, End=255 };
enum class Side : uint8_t { Bid=0, Ask=1 };   // NEW

struct MdEvent {
  EventType type{EventType::Add};
  Side side{Side::Bid};                       // NEW

  uint32_t instrument_id{0};
  uint64_t seq{0};

  int64_t  price{0};
  int32_t  qty{0};

  uint64_t t0_ns{0};
};
}