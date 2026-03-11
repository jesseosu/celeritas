#include "engine/matching_engine.hpp"
#include "common/timestamp.hpp"
#include "celeritas/exchange/Order.hpp"
#include "celeritas/exchange/Trade.hpp"

namespace llp::engine {

using llp::exchange::ExecType;
using llp::exchange::ExecReport;

MatchingEngine::MatchingEngine(InQueue&   in_q,
                               ExecQueue& exec_q,
                               MdQueue&   md_q,
                               llp::metrics::LatencyTracker& lat)
    : in_q_(in_q), exec_q_(exec_q), md_q_(md_q), lat_(lat) {}

EngineStats MatchingEngine::run() {
    OrderRequest req{};

    while (true) {
        if (!in_q_.pop(req)) {
            llp::time::cpu_relax();
            continue;
        }

        if (req.type == RequestType::End) break;

        // Measure only engine processing time, not queue wait time
        const uint64_t t_start = llp::time::now_ns();

        if (req.type == RequestType::NewOrder) {
            handle_new_order(req);
        } else if (req.type == RequestType::Cancel) {
            handle_cancel(req);
        }

        const uint64_t t_end = llp::time::now_ns();
        lat_.record(t_end - t_start);

        stats_.processed++;
    }

    return stats_;
}

void MatchingEngine::handle_new_order(const OrderRequest& req) {
    const int prev_bid = book_.best_bid();
    const int prev_ask = book_.best_ask();

    celeritas::Order o{};
    o.id        = static_cast<celeritas::OrderId>(req.order_id);
    o.side      = (req.side == Side::Buy) ? celeritas::Side::Buy : celeritas::Side::Sell;
    o.price     = static_cast<int>(req.price);
    o.quantity  = static_cast<int>(req.qty);
    o.timestamp = llp::time::now_ns();

    order_symbol_[req.order_id] = req.symbol;

    // Ack first
    publish_exec(ExecType::Ack,
                 req.order_id, 0,
                 req.price, req.qty, req.qty,
                 req.t0_ns);

    // Match
    auto trades = book_.add_limit(o);

    int leaves = req.qty;
    for (const auto& t : trades) {
        const int fill_qty = t.quantity;
        leaves -= fill_qty;

        const uint64_t matched_id =
            (static_cast<celeritas::OrderId>(req.order_id) == t.buy_id)
            ? static_cast<uint64_t>(t.sell_id)
            : static_cast<uint64_t>(t.buy_id);

        publish_exec(ExecType::Fill,
                     req.order_id,
                     matched_id,
                     static_cast<int64_t>(t.price),
                     fill_qty,
                     std::max(0, leaves),
                     req.t0_ns);
    }

    maybe_publish_md(req.symbol, prev_bid, prev_ask);
}

void MatchingEngine::handle_cancel(const OrderRequest& req) {
    const int prev_bid = book_.best_bid();
    const int prev_ask = book_.best_ask();

    bool cancelled = book_.cancel(req.order_id);

    if (cancelled) {
        order_symbol_.erase(req.order_id);
        publish_exec(ExecType::CancelAck,
                     req.order_id, 0, 0, 0, 0,
                     req.t0_ns);
        maybe_publish_md(req.symbol, prev_bid, prev_ask);
    } else {
        publish_exec(ExecType::Reject,
                     req.order_id, 0, 0, 0, 0,
                     req.t0_ns);
    }
}

void MatchingEngine::publish_exec(ExecType type,
                                  uint64_t order_id,
                                  uint64_t matched_id,
                                  int64_t  price,
                                  int32_t  qty,
                                  int32_t  leaves,
                                  uint64_t t0_ns) {
    ExecReport r{};
    r.type             = type;
    r.order_id         = order_id;
    r.matched_order_id = matched_id;
    r.price            = price;
    r.qty              = qty;
    r.leaves_qty       = leaves;
    r.t0_ns            = t0_ns;
    r.t_eng_ns         = llp::time::now_ns();

    while (!exec_q_.push(r)) llp::time::cpu_relax();
    stats_.exec_reports++;
}

void MatchingEngine::maybe_publish_md(uint32_t symbol,
                                      int prev_bid,
                                      int prev_ask) {
    const int new_bid = book_.best_bid();
    const int new_ask = book_.best_ask();

    if (new_bid == prev_bid && new_ask == prev_ask) return;

    MarketDataEvent md{};
    md.symbol       = symbol;
    md.best_bid_px  = new_bid;
    md.best_bid_qty = book_.bid_size();
    md.best_ask_px  = new_ask;
    md.best_ask_qty = book_.ask_size();
    md.ts_ns        = llp::time::now_ns();

    while (!md_q_.push(md)) llp::time::cpu_relax();
    stats_.md_updates++;
}

}