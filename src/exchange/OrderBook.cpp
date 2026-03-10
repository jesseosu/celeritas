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

  if (incoming.quantity <= 0 || incoming.price <= 0) return trades;

  if (incoming.side == Side::Buy) {
    while (incoming.quantity > 0 && !asks_.empty()) {
      auto best = asks_.begin();
      const int ask_px = best->first;
      if (ask_px > incoming.price) break;

      auto& fifo = best->second;
      while (incoming.quantity > 0 && !fifo.empty()) {
        Order& resting = fifo.front();
        const int fill = std::min(resting.quantity, incoming.quantity);

        trades.push_back(Trade{
          incoming.id, resting.id, ask_px, fill, now_ns()
        });

        resting.quantity -= fill;
        incoming.quantity -= fill;

        if (resting.quantity == 0) {
          order_index_.erase(resting.id);
          fifo.pop_front();
        }
      }
      if (fifo.empty()) asks_.erase(best);
    }

    if (incoming.quantity > 0) {
      order_index_[incoming.id] = {Side::Buy, incoming.price};
      bids_[incoming.price].push_back(incoming);
    }

  } else {
    while (incoming.quantity > 0 && !bids_.empty()) {
      auto best = bids_.begin();
      const int bid_px = best->first;
      if (bid_px < incoming.price) break;

      auto& fifo = best->second;
      while (incoming.quantity > 0 && !fifo.empty()) {
        Order& resting = fifo.front();
        const int fill = std::min(resting.quantity, incoming.quantity);

        trades.push_back(Trade{
          resting.id, incoming.id, bid_px, fill, now_ns()
        });

        resting.quantity -= fill;
        incoming.quantity -= fill;

        if (resting.quantity == 0) {
          order_index_.erase(resting.id);
          fifo.pop_front();
        }
      }
      if (fifo.empty()) bids_.erase(best);
    }

    if (incoming.quantity > 0) {
      order_index_[incoming.id] = {Side::Sell, incoming.price};
      asks_[incoming.price].push_back(incoming);
    }
  }

  return trades;
}

bool OrderBook::cancel(OrderId order_id) {
  auto it = order_index_.find(order_id);
  if (it == order_index_.end()) return false;

  const Side side  = it->second.side;
  const int  price = it->second.price;
  order_index_.erase(it);

  if (side == Side::Buy) {
    auto level = bids_.find(price);
    if (level == bids_.end()) return false;

    auto& fifo = level->second;
    for (auto oit = fifo.begin(); oit != fifo.end(); ++oit) {
      if (oit->id == order_id) {
        fifo.erase(oit);
        if (fifo.empty()) bids_.erase(level);
        return true;
      }
    }
  } else {
    auto level = asks_.find(price);
    if (level == asks_.end()) return false;

    auto& fifo = level->second;
    for (auto oit = fifo.begin(); oit != fifo.end(); ++oit) {
      if (oit->id == order_id) {
        fifo.erase(oit);
        if (fifo.empty()) asks_.erase(level);
        return true;
      }
    }
  }

  return false;
}

} // namespace celeritas