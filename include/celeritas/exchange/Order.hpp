#pragma once
#include <cstdint>
#include "celeritas/common/Types.hpp"

namespace celeritas {

struct Order {
  OrderId id{};
  Side side{Side::Buy};
  int price{0};              // limit price in ticks
  int quantity{0};           // remaining quantity
  std::uint64_t timestamp{0};
};

}