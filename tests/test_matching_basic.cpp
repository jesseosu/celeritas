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

  // Resting sell, no cross
  auto t1 = book.add_limit(mk(1, Side::Sell, 105, 10));
  assert(t1.empty());
  assert(book.best_ask() == 105);
  assert(book.ask_size() == 10);
  assert(book.best_bid() == 0);

  // Resting buy, no cross
  auto t2 = book.add_limit(mk(2, Side::Buy, 100, 5));
  assert(t2.empty());
  assert(book.best_bid() == 100);
  assert(book.bid_size() == 5);

  // Crossing buy — fully matches the resting sell at 105
  auto t3 = book.add_limit(mk(3, Side::Buy, 110, 10));
  assert(t3.size() == 1);
  assert(t3[0].price    == 105);
  assert(t3[0].quantity == 10);
  assert(t3[0].buy_id   == 3);
  assert(t3[0].sell_id  == 1);
  assert(book.best_ask() == 0);
  assert(book.ask_size() == 0);

  // Earlier bid still resting
  assert(book.best_bid() == 100);
  assert(book.bid_size() == 5);

  std::cout << "test_matching_basic passed\n";
  return 0;
}