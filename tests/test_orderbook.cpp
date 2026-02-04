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

  // 1) Add a sell that should rest (no bids)
  {
    auto trades = book.add_limit(mk(1, Side::Sell, 105, 10));
    assert(trades.empty());
    assert(book.best_ask() == 105);
    assert(book.ask_size() == 10);
    assert(book.best_bid() == 0);
  }

  // 2) Add a buy that does NOT cross (price too low) -> rests on bid
  {
    auto trades = book.add_limit(mk(2, Side::Buy, 100, 7));
    assert(trades.empty());
    assert(book.best_bid() == 100);
    assert(book.bid_size() == 7);
    assert(book.best_ask() == 105);
    assert(book.ask_size() == 10);
  }

  // 3) Add a crossing buy that fully matches the ask at 105
  {
    auto trades = book.add_limit(mk(3, Side::Buy, 110, 10));
    assert(trades.size() == 1);
    assert(trades[0].price == 105);
    assert(trades[0].quantity == 10);
    assert(trades[0].buy_id == 3);
    assert(trades[0].sell_id == 1);

    // ask consumed
    assert(book.best_ask() == 0);
    assert(book.ask_size() == 0);

    // earlier bid still there
    assert(book.best_bid() == 100);
    assert(book.bid_size() == 7);
  }

  // 4) Partial fill: add sell that crosses best bid 100 but bigger qty
  {
    auto trades = book.add_limit(mk(4, Side::Sell, 95, 20));
    assert(trades.size() == 1);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 7);
    assert(trades[0].buy_id == 2);
    assert(trades[0].sell_id == 4);

    // bid consumed, remaining sell should rest at 95 with qty 13
    assert(book.best_bid() == 0);
    assert(book.bid_size() == 0);
    assert(book.best_ask() == 95);
    assert(book.ask_size() == 13);
  }

  std::cout << "orderbook tests passed\n";
  return 0;
}
