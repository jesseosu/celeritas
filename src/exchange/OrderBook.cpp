#include "celeritas/exchange/OrderBook.hpp"
#include "celeritas/common/Time.hpp"

#include <algorithm>

namespace celeritas {

int OrderBook::total_qty(const std::map<int, std::deque<Order>>& levels) {
  int sum = 0;
  for (const auto& [price, q] : levels) {
    (void)price;
    for (const auto& o : q) sum += o.quantity;
  }
  return sum;
}

int OrderBook::best_bid() const {
  if (bids_.empty()) return 0;
  return bids_.begin()->first;
}

int OrderBook::best_ask() const {
  if (asks_.empty()) return 0;
  return asks_.begin()->first;
}

int OrderBook::bid_size() const {
  // Need a const-compatible total (we’ll compute directly here)
  int sum = 0;
  for (const auto& [price, q] : bids_) {
    (void)price;
    for (const auto& o : q) sum += o.quantity;
  }
  return sum;
}

int OrderBook::ask_size() const {
  return total_qty(asks_);
}

std::vector<Trade> OrderBook::add_limit(const Order& in) {
  std::vector<Trade> trades;
  Order incoming = in;

  // Basic input guards (keep it strict for now)
  if (incoming.quantity <= 0 || incoming.price <= 0) {
    return trades;
  }

  if (incoming.side == Side::Buy) {
    // Match against best asks while price crosses
    while (incoming.quantity > 0 && !asks_.empty()) {
      auto best = asks_.begin(); // lowest ask
      const int ask_px = best->first;

      if (ask_px > incoming.price) break; // no cross

      auto& fifo = best->second;
      while (incoming.quantity > 0 && !fifo.empty()) {
        Order& resting = fifo.front();

        const int fill = std::min(resting.quantity, incoming.quantity);

        trades.push_back(Trade{
          /*buy_id*/  incoming.id,
          /*sell_id*/ resting.id,
          /*price*/   ask_px,
          /*quantity*/fill,
          /*timestamp*/ now_ns()
        });

        resting.quantity -= fill;
        incoming.quantity -= fill;

        if (resting.quantity == 0) fifo.pop_front();
      }

      if (fifo.empty()) asks_.erase(best);
    }

    // Rest remainder on bids at its limit price
    if (incoming.quantity > 0) {
      bids_[incoming.price].push_back(incoming);
    }

  } else { // Side::Sell
    // Match against best bids while price crosses
    while (incoming.quantity > 0 && !bids_.empty()) {
      auto best = bids_.begin(); // highest bid (due to comparator)
      const int bid_px = best->first;

      if (bid_px < incoming.price) break; // no cross

      auto& fifo = best->second;
      while (incoming.quantity > 0 && !fifo.empty()) {
        Order& resting = fifo.front();

        const int fill = std::min(resting.quantity, incoming.quantity);

        trades.push_back(Trade{
          /*buy_id*/  resting.id,
          /*sell_id*/ incoming.id,
          /*price*/   bid_px,
          /*quantity*/fill,
          /*timestamp*/ now_ns()
        });

        resting.quantity -= fill;
        incoming.quantity -= fill;

        if (resting.quantity == 0) fifo.pop_front();
      }

      if (fifo.empty()) bids_.erase(best);
    }

    // Rest remainder on asks at its limit price
    if (incoming.quantity > 0) {
      asks_[incoming.price].push_back(incoming);
    }
  }

  return trades;
}

}
