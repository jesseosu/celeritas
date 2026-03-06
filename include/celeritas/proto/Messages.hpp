#pragma once

#include <cstdint>
#include <type_traits>

#include "celeritas/common/Types.hpp"   // OrderId, Side

namespace celeritas::proto {

// -----------------------------
// Protocol constants
// -----------------------------
inline constexpr std::uint32_t kMagic = 0x43454C45; // 'C''E''L''E'
inline constexpr std::uint16_t kVersionMajor = 1;
inline constexpr std::uint16_t kVersionMinor = 0;

// Keep message types stable. Never reuse numeric values.
enum class MsgType : std::uint16_t {
  NewOrder   = 1,
  Cancel     = 2,
  Replace    = 3,   // reserved for later steps
  Heartbeat  = 4    // reserved
};

// Result codes for acks/rejects (reserved for later steps)
enum class RejectReason : std::uint16_t {
  None            = 0,
  UnknownType     = 1,
  BadLength       = 2,
  BadVersion      = 3,
  InvalidField    = 4,
  UnknownOrderId  = 5
};

// -----------------------------
// Packed wire structs
// -----------------------------
#if defined(_MSC_VER)
  #pragma pack(push, 1)
  #define CELERITAS_PACKED
#else
  #define CELERITAS_PACKED __attribute__((packed))
#endif

struct CELERITAS_PACKED MsgHeader {
  std::uint32_t magic;        // kMagic
  std::uint16_t ver_major;    // kVersionMajor
  std::uint16_t ver_minor;    // kVersionMinor
  std::uint16_t type;         // MsgType as u16
  std::uint16_t length;       // total bytes = header + payload
  std::uint64_t seq;          // monotonically increasing per stream
  std::uint64_t ts_ns;        // sender timestamp (ns)
};

static_assert(std::is_trivially_copyable_v<MsgHeader>);
static_assert(sizeof(MsgHeader) == 28, "MsgHeader size must be stable (28 bytes)");

// ---- New Order (LIMIT only in Phase 2 MVP) ----
// Semantics: add a resting order; matching happens in engine.
struct CELERITAS_PACKED NewOrder {
  MsgHeader hdr;              // type = MsgType::NewOrder
  OrderId   order_id;
  std::uint8_t side;          // Side as u8 (Buy=0, Sell=1)
  std::uint8_t flags;         // reserved (e.g., IOC/FOK later)
  std::uint16_t reserved0;    // reserved
  std::int32_t price;         // ticks (must be > 0 for limit)
  std::int32_t qty;           // > 0
  std::uint32_t symbol;       // numeric symbol id (Phase 2 MVP)
  std::uint32_t reserved1;    // reserved
};

static_assert(std::is_trivially_copyable_v<NewOrder>);
// Payload after header is 28 bytes -> total 56 bytes
static_assert(sizeof(NewOrder) == sizeof(MsgHeader) + 28, "NewOrder size mismatch");

// ---- Cancel ----
// Semantics: cancel resting order by id (symbol included for routing)
struct CELERITAS_PACKED Cancel {
  MsgHeader hdr;              // type = MsgType::Cancel
  OrderId   order_id;
  std::uint32_t symbol;
  std::uint32_t reserved0;
};

static_assert(std::is_trivially_copyable_v<Cancel>);
static_assert(sizeof(Cancel) == sizeof(MsgHeader) + 16, "Cancel size mismatch");

// ---- Replace (reserved for later) ----
// Replace means modify price/qty while preserving time priority rules you define.
struct CELERITAS_PACKED Replace {
  MsgHeader hdr;              // type = MsgType::Replace
  OrderId   order_id;
  std::int32_t new_price;
  std::int32_t new_qty;
  std::uint32_t symbol;
  std::uint32_t reserved0;
};

static_assert(std::is_trivially_copyable_v<Replace>);
static_assert(sizeof(Replace) == sizeof(MsgHeader) + 24, "Replace size mismatch");

#if defined(_MSC_VER)
  #pragma pack(pop)
#endif

#undef CELERITAS_PACKED

// -----------------------------
// Helpers (header initialization)
// -----------------------------
inline MsgHeader make_header(MsgType t, std::uint16_t total_len,
                             std::uint64_t seq, std::uint64_t ts_ns) {
  MsgHeader h{};
  h.magic = kMagic;
  h.ver_major = kVersionMajor;
  h.ver_minor = kVersionMinor;
  h.type = static_cast<std::uint16_t>(t);
  h.length = total_len;
  h.seq = seq;
  h.ts_ns = ts_ns;
  return h;
}

}
