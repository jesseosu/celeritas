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

  // Rest a buy 100 x 5
  book.add_limit(mk(1, Side::Buy, 100, 5));
  assert(book.bid_size() == 5);

  // Sell 20 at 95 — crosses bid at 100, partial fill of 5, remainder 15 rests at 95
  auto trades = book.add_limit(mk(2, Side::Sell, 95, 20));
  assert(trades.size() == 1);
  assert(trades[0].price    == 100);
  assert(trades[0].quantity == 5);
  assert(trades[0].buy_id   == 1);
  assert(trades[0].sell_id  == 2);

  // Bid fully consumed
  assert(book.best_bid() == 0);
  assert(book.bid_size() == 0);

  // Remainder rests on ask at 95 with qty 15
  assert(book.best_ask() == 95);
  assert(book.ask_size() == 15);

  std::cout << "test_matching_partial passed\n";
  return 0;
}