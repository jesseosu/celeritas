#pragma once
#include <deque>
#include <vector>
#include "Order.hpp"
#include "Trade.hpp"

namespace celeritas {

class OrderBook {
public:
  std::vector<Trade> add_order(const Order& order);

private:
  std::deque<Order> bids_;
  std::deque<Order> asks_;
};

}
