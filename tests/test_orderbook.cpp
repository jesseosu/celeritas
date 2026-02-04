#include "celeritas/exchange/OrderBook.hpp"
#include <cassert>
#include <iostream>

using namespace celeritas;

int main() {
  OrderBook book;

  Order sell{1, Side::Sell, 100, 10, now_ns()};
  Order buy{2, Side::Buy, 100, 10, now_ns()};

  auto t1 = book.add_order(sell);
  assert(t1.empty());

  auto t2 = book.add_order(buy);
  assert(t2.size() == 1);
  assert(t2[0].price == 100);
  assert(t2[0].quantity == 10);

  std::cout << "orderbook tests passed\n";
}

