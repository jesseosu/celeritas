#include "celeritas/exchange/OrderBook.hpp"
#include "celeritas/common/Time.hpp"

namespace celeritas {

std::vector<Trade> OrderBook::add_order(const Order& incoming) {
  std::vector<Trade> trades;

  if (incoming.side == Side::Buy) {
    while (!asks_.empty() &&
           incoming.quantity > 0 &&
           asks_.front().price <= incoming.price) {

      Order& resting = asks_.front();
      int qty = std::min(resting.quantity, incoming.quantity);

      trades.push_back({
        incoming.id,
        resting.id,
        resting.price,
        qty,
        now_ns()
      });

      resting.quantity -= qty;
      if (resting.quantity == 0)
        asks_.pop_front();
    }

    if (incoming.quantity > 0)
      bids_.push_back(incoming);
  }
  else {
    while (!bids_.empty() &&
           incoming.quantity > 0 &&
           bids_.front().price >= incoming.price) {

      Order& resting = bids_.front();
      int qty = std::min(resting.quantity, incoming.quantity);

      trades.push_back({
        resting.id,
        incoming.id,
        resting.price,
        qty,
        now_ns()
      });

      resting.quantity -= qty;
      if (resting.quantity == 0)
        bids_.pop_front();
    }

    if (incoming.quantity > 0)
      asks_.push_back(incoming);
  }

  return trades;
}

}
