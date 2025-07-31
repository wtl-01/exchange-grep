#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <unordered_map>
#include <list>
#include "order_structures.hpp"

using namespace std;


#ifndef SIGNALS_HPP
#define SIGNALS_HPP

enum SignalType {
BBO_MOVEMENT_TRIGGER,
SPECIFIC_TRADE_TRIGGER,
SPECIFIC_TRADE_CANCEL_TRIGGER,
};

//long double calc_theo_template(BestBidOffer bbo) {
//    long double bid_p = static_cast<long double>(bbo.best_bid_price);
//    long double bid_q = static_cast<long double>(bbo.best_bid_amount);
//    long double ask_p = static_cast<long double>(bbo.best_ask_price);
//    long double ask_q = static_cast<long double>(bbo.best_ask_amount);
//
//    return ((bid_p * ask_q) + (ask_p * bid_q)) / (bid_q + ask_q);
//};


enum class BBOSignalFlags : uint64_t {
    SIDE_BIT       = 1ULL << 0,  // 0 for bid, 1 for ask
    BBO_SHIFT      = 1ULL << 1,  // BBO shifted one level
    LARGE_ORDER    = 1ULL << 2,  // Large order (100-3000) executed
    THEO_CROSS     = 1ULL << 3,   // Order crosses theo by 0.5 ticks

    // TODO: Add More Signals Later
    RESERVED_4     = 1ULL << 4,
    RESERVED_5     = 1ULL << 5,
    RESERVED_6     = 1ULL << 6,
    RESERVED_7     = 1ULL << 7
};

struct DataRecord {
    BestBidOffer bbo;
    uint64_t theo;
    uint32_t quantity;
    int64_t price;
    Side side;
};

struct SignalInstance {
    string hr_ticker;
    uint64_t buy_signals;
    uint64_t sell_signals;
};

class SignalUtils {
public:
    static bool hasSignal(const uint64_t& signal_flags, BBOSignalFlags flag) {
        return (signal_flags & static_cast<uint64_t>(flag)) == static_cast<uint64_t>(flag);
    }

    static void setSignal(uint64_t& signal_flags, BBOSignalFlags flag) {
        signal_flags |= static_cast<uint64_t>(flag);
    }

    static void clearSignal(uint64_t& signal_flags, BBOSignalFlags flag) {
        signal_flags &= ~static_cast<uint64_t>(flag);
    }

    static void verifySignalFlags(uint64_t& cur_entry_sig, BBOSignalFlags flag) {
        if (flag == BBOSignalFlags::LARGE_ORDER) {

        }
    }

    static bool processLargeOrderSignal(uint64_t& cur_entry_sig, uint32_t notional, uint32_t lim_min = 100, uint32_t lim_max = 3000) {
        if (notional >= lim_min && notional <= lim_max) {

        }
    }
};

class SignalManager {
    //TODO: Implement proper signal capture. Write signal activations to csv

};

#endif //SIGNALS_HPP
