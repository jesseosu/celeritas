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

  // Add two resting bids
  book.add_limit(mk(1, Side::Buy, 100, 10));
  book.add_limit(mk(2, Side::Buy, 100, 5));
  assert(book.bid_size() == 15);
  assert(book.best_bid() == 100);

  // Cancel order 1
  bool ok = book.cancel(1);
  assert(ok);
  assert(book.bid_size() == 5);   // only order 2 remains
  assert(book.best_bid() == 100); // price level still exists

  // Cancel order 2
  ok = book.cancel(2);
  assert(ok);
  assert(book.bid_size() == 0);
  assert(book.best_bid() == 0);   // level gone

  // Cancel non-existent order returns false
  ok = book.cancel(999);
  assert(!ok);

  // Cancel on ask side
  book.add_limit(mk(3, Side::Sell, 105, 8));
  assert(book.ask_size() == 8);
  ok = book.cancel(3);
  assert(ok);
  assert(book.ask_size() == 0);
  assert(book.best_ask() == 0);

  std::cout << "test_matching_cancel passed\n";
  return 0;
}