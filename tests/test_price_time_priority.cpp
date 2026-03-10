#include "celeritas/exchange/OrderBook.hpp"
#include "celeritas/common/Time.hpp"
#include <cassert>
#include <iostream>

using namespace celeritas;

static Order mk(OrderId id, Side s, int px, int qty) {
  return Order{ id, s, px, qty, now_ns() };
}

int main() {
  OrderBook book;

  // Two bids at same price — order 1 arrives first
  book.add_limit(mk(1, Side::Buy, 100, 5));
  book.add_limit(mk(2, Side::Buy, 100, 5));
  assert(book.bid_size() == 10);

  // Sell 5 at 100 — must fill order 1 first (time priority)
  auto trades = book.add_limit(mk(3, Side::Sell, 100, 5));
  assert(trades.size() == 1);
  assert(trades[0].quantity == 5);
  assert(trades[0].buy_id   == 1); // order 1 filled, NOT order 2
  assert(trades[0].sell_id  == 3);

  // Order 2 still fully resting
  assert(book.bid_size() == 5);
  assert(book.best_bid() == 100);

  // Now sell 5 more — must fill order 2
  auto trades2 = book.add_limit(mk(4, Side::Sell, 100, 5));
  assert(trades2.size() == 1);
  assert(trades2[0].buy_id == 2);
  assert(book.bid_size() == 0);

  // Better price priority — higher bid matches first
  book.add_limit(mk(5, Side::Buy, 98, 10));
  book.add_limit(mk(6, Side::Buy, 100, 10)); // better price
  book.add_limit(mk(7, Side::Buy, 99, 10));

  // Sell at 95 — should match 100 first, then 99, then 98
  auto trades3 = book.add_limit(mk(8, Side::Sell, 95, 30));
  assert(trades3.size() == 3);
  assert(trades3[0].price == 100); // best price first
  assert(trades3[1].price == 99);
  assert(trades3[2].price == 98);

  std::cout << "test_price_time_priority passed\n";
  return 0;
}