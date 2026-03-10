#pragma once
#include <cstdint>
#include <unordered_map>

#include "engine/order_request.hpp"
#include "engine/market_data.hpp"
#include "exchange/exec_report.hpp"
#include "ipc/spsc_ring_buffer.hpp"
#include "metrics/latency.hpp"

// Pull in the existing celeritas order book
#include "celeritas/exchange/OrderBook.hpp"

namespace llp::engine {

constexpr size_t EXEC_Q_CAP = 1u << 16;
constexpr size_t MD_Q_CAP   = 1u << 16;
constexpr size_t IN_Q_CAP   = 1u << 16;

using InQueue   = llp::ipc::SpscRingBuffer<OrderRequest,          IN_Q_CAP>;
using ExecQueue = llp::ipc::SpscRingBuffer<llp::exchange::ExecReport, EXEC_Q_CAP>;
using MdQueue   = llp::ipc::SpscRingBuffer<MarketDataEvent,        MD_Q_CAP>;

struct EngineStats {
    uint64_t processed{0};
    uint64_t exec_reports{0};
    uint64_t md_updates{0};
};

class MatchingEngine {
public:
    MatchingEngine(InQueue&   in_q,
                   ExecQueue& exec_q,
                   MdQueue&   md_q,
                   llp::metrics::LatencyTracker& lat);

    EngineStats run(); // blocks until End sentinel

private:
    void handle_new_order(const OrderRequest& req);
    void handle_cancel(const OrderRequest& req);

    void publish_exec(llp::exchange::ExecType type,
                      uint64_t order_id,
                      uint64_t matched_id,
                      int64_t  price,
                      int32_t  qty,
                      int32_t  leaves,
                      uint64_t t0_ns);

    void maybe_publish_md(uint32_t symbol,
                          int prev_bid, int prev_ask);

    InQueue&   in_q_;
    ExecQueue& exec_q_;
    MdQueue&   md_q_;
    llp::metrics::LatencyTracker& lat_;

    celeritas::OrderBook book_;

    // Track resting orders: order_id -> symbol (for cancel routing)
    std::unordered_map<uint64_t, uint32_t> order_symbol_;

    EngineStats stats_;
};

}