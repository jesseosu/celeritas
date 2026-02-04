#pragma once
#include <cstdint>

namespace celeritas {

using OrderId = std::uint64_t;

enum class Side : std::uint8_t {
  Buy = 0,
  Sell = 1
};

}