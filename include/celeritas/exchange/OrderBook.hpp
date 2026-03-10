#pragma once
#include <deque>
#include <map>
#include <unordered_map>
#include <vector>
#include "celeritas/exchange/Order.hpp"
#include "celeritas/exchange/Trade.hpp"

namespace celeritas {

class OrderBook {
public:
  // Adds a LIMIT order and returns resulting trades.
  // If not fully filled, remainder rests on the book.
  std::vector<Trade> add_limit(const Order& order);

  // Cancel a resting order by id. Returns true if found and removed.
  bool cancel(OrderId order_id);

  // Simple introspection helpers for tests/debug
  int best_bid() const;   // returns 0 if no bids
  int best_ask() const;   // returns 0 if no asks
  int bid_size() const;   // total resting bid qty
  int ask_size() const;   // total resting ask qty

private:
  // price -> FIFO queue
  std::map<int, std::deque<Order>> asks_;
  std::map<int, std::deque<Order>, std::greater<int>> bids_;

  // order_id -> {side, price} for O(1) cancel lookup
  struct OrderLocation {
    Side side;
    int  price;
  };
  std::unordered_map<OrderId, OrderLocation> order_index_;

  static int total_qty(const std::map<int, std::deque<Order>>& levels);
};

} // namespace celeritas