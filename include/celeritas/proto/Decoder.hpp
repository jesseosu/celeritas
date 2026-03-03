#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>

#include "celeritas/proto/Messages.hpp"

namespace celeritas::proto {

struct DecodeError {
  RejectReason reason{RejectReason::None};
  std::string_view message{};
};

// Typed result of decoding one message
using DecodedMsg = std::variant<NewOrder, Cancel, Replace>;

struct DecodeResult {
  std::optional<DecodedMsg> msg;
  std::optional<DecodeError> err;
  std::size_t bytes_consumed{0};
};

std::uint16_t expected_length(MsgType t);

// Decode exactly one message from a raw buffer.
// - If successful: msg is set and bytes_consumed == hdr.length
// - If not enough bytes: returns {null msg, null err, 0} (caller should read more)
// - If invalid: err is set (and bytes_consumed == 0)
DecodeResult decode_one(const std::uint8_t* data, std::size_t len);

} // namespace celeritas::proto
