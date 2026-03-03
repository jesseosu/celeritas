#pragma once
#include <cstdint>

namespace llp::exchange {

enum class ExecType : uint8_t { Ack=0, Fill=1, End=255 };

struct ExecReport {
  ExecType type{ExecType::Ack};
  uint64_t seq{0};
  uint64_t t0_ns{0};   // from original event (producer time)
  uint64_t t_eng_ns{0}; // when engine created report
};
}