#pragma once
#include <cstdint>
#include "celeritas/common/Types.hpp"

namespace celeritas {

struct Order {
  OrderId id;
  Side side;
  int price;
  int quantity;
  std::uint64_t timestamp;
};

}