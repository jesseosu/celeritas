#pragma once
#include <cstdint>
#include "celeritas/common/Types.hpp"

namespace celeritas {

struct Trade {
  OrderId buy_id{};
  OrderId sell_id{};
  int price{0};
  int quantity{0};
  std::uint64_t timestamp{0};
};

}
