#pragma once
#include <cstdint>
#include "celeritas/common/Types.hpp"

namespace celeritas {

struct Trade {
  OrderId buy_id;
  OrderId sell_id;
  int price;
  int quantity;
  std::uint64_t timestamp;
};

}
