#include <cassert>
#include "engine/order_book_l2.hpp"
#include "feed/md_event.hpp"

int main() {
  llp::engine::OrderBookL2 b;
  llp::feed::MdEvent e{};
  e.instrument_id = 1;
  e.type = llp::feed::EventType::Add;
  e.side = llp::feed::Side::Bid;
  e.price = 100;
  e.qty = 5;
  b.apply(e);

  auto c1 = b.checksum();
  e.qty = 0;
  e.type = llp::feed::EventType::Cancel;
  b.apply(e);
  auto c2 = b.checksum();

  assert(c1 != c2);
  return 0;
}
