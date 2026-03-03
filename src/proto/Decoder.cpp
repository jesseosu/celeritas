#include "celeritas/proto/Decoder.hpp"

#include <cstring>   // std::memcpy

namespace celeritas::proto {

static bool is_supported_type(std::uint16_t t) {
  return t == static_cast<std::uint16_t>(MsgType::NewOrder) ||
         t == static_cast<std::uint16_t>(MsgType::Cancel) ||
         t == static_cast<std::uint16_t>(MsgType::Replace) ||
         t == static_cast<std::uint16_t>(MsgType::Heartbeat);
}

std::uint16_t expected_length(MsgType t) {
  switch (t) {
    case MsgType::NewOrder:  return static_cast<std::uint16_t>(sizeof(NewOrder));
    case MsgType::Cancel:    return static_cast<std::uint16_t>(sizeof(Cancel));
    case MsgType::Replace:   return static_cast<std::uint16_t>(sizeof(Replace));
    case MsgType::Heartbeat: return static_cast<std::uint16_t>(sizeof(MsgHeader)); // header-only
    default:                 return 0;
  }
}

static DecodeResult fail(RejectReason r, std::string_view msg) {
  DecodeResult out;
  out.err = DecodeError{r, msg};
  out.bytes_consumed = 0;
  return out;
}

DecodeResult decode_one(const std::uint8_t* data, std::size_t len) {
  DecodeResult out;

  if (!data || len < sizeof(MsgHeader)) {
    // Not enough bytes yet
    out.bytes_consumed = 0;
    return out;
  }

  MsgHeader hdr{};
  std::memcpy(&hdr, data, sizeof(MsgHeader));

  // Validate header basics
  if (hdr.magic != kMagic) {
    return fail(RejectReason::InvalidField, "bad magic");
  }
  if (hdr.ver_major != kVersionMajor) {
    return fail(RejectReason::BadVersion, "unsupported major version");
  }
  if (!is_supported_type(hdr.type)) {
    return fail(RejectReason::UnknownType, "unknown msg type");
  }

  const auto type = static_cast<MsgType>(hdr.type);
  const std::uint16_t want_len = expected_length(type);

  if (want_len == 0) {
    return fail(RejectReason::BadLength, "unknown expected length");
  }
  if (hdr.length != want_len) {
    return fail(RejectReason::BadLength, "length mismatch");
  }

  if (len < hdr.length) {
    // Partial buffer: caller should read more
    out.bytes_consumed = 0;
    return out;
  }

  // Decode payload
  if (type == MsgType::Heartbeat) {
    // Header-only message: we don’t store it as a variant yet (reserved for later).
    // Treat as invalid for now (or you can add a Heartbeat type in the variant).
    return fail(RejectReason::UnknownType, "heartbeat not supported yet");
  }

  if (type == MsgType::NewOrder) {
    NewOrder m{};
    std::memcpy(&m, data, sizeof(NewOrder));

    // Minimal field validation
    if (m.qty <= 0 || m.price <= 0) return fail(RejectReason::InvalidField, "invalid qty/price");
    if (m.side != static_cast<std::uint8_t>(celeritas::Side::Buy) &&
        m.side != static_cast<std::uint8_t>(celeritas::Side::Sell)) {
      return fail(RejectReason::InvalidField, "invalid side");
    }

    out.msg = m;
    out.bytes_consumed = sizeof(NewOrder);
    return out;
  }

  if (type == MsgType::Cancel) {
    Cancel m{};
    std::memcpy(&m, data, sizeof(Cancel));
    out.msg = m;
    out.bytes_consumed = sizeof(Cancel);
    return out;
  }

  if (type == MsgType::Replace) {
    Replace m{};
    std::memcpy(&m, data, sizeof(Replace));
    if (m.new_qty <= 0 || m.new_price <= 0) return fail(RejectReason::InvalidField, "invalid new qty/price");
    out.msg = m;
    out.bytes_consumed = sizeof(Replace);
    return out;
  }

  return fail(RejectReason::UnknownType, "unsupported msg type");
}

} // namespace celeritas::proto
