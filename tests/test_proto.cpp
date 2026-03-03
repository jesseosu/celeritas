#include "celeritas/proto/Decoder.hpp"
#include "celeritas/common/Time.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

using namespace celeritas;
using namespace celeritas::proto;

static std::vector<std::uint8_t> to_bytes(const void* p, std::size_t n) {
  std::vector<std::uint8_t> out(n);
  std::memcpy(out.data(), p, n);
  return out;
}

int main() {
  // Build a valid NewOrder message in memory
  NewOrder no{};
  no.hdr = make_header(MsgType::NewOrder,
                       static_cast<std::uint16_t>(sizeof(NewOrder)),
                       /*seq*/1,
                       /*ts*/now_ns());
  no.order_id = 42;
  no.side = static_cast<std::uint8_t>(Side::Buy);
  no.flags = 0;
  no.price = 100;
  no.qty = 10;
  no.symbol = 1;

  auto bytes = to_bytes(&no, sizeof(NewOrder));
  auto r = decode_one(bytes.data(), bytes.size());
  assert(!r.err.has_value());
  assert(r.msg.has_value());
  assert(r.bytes_consumed == sizeof(NewOrder));

  {
    const auto& v = *r.msg;
    assert(std::holds_alternative<NewOrder>(v));
    const auto& decoded = std::get<NewOrder>(v);
    assert(decoded.order_id == 42);
    assert(decoded.price == 100);
    assert(decoded.qty == 10);
    assert(decoded.side == static_cast<std::uint8_t>(Side::Buy));
  }

  // Invalid magic
  bytes[0] ^= 0xFF;
  auto r2 = decode_one(bytes.data(), bytes.size());
  assert(r2.err.has_value());
  assert(r2.bytes_consumed == 0);

  std::cout << "proto decoder tests passed\n";
  return 0;
}
