

#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <map>
#include <queue>
#include <memory>
#include <cstdint>
#include <ctime>
#include <list>
#include <unordered_map>

#include "order_structures.hpp"
#include "../include/cfepitch.h"

using OrderPtr = std::shared_ptr<Order>;
using OrderQueue = std::list<OrderPtr>;
using BidQueue = std::map<int64_t, OrderQueue, std::greater<int64_t>>;
using AskQueue = std::map<int64_t, OrderQueue, std::less<int64_t>>;


class OrderBook {


    public:
        OrderBook();
        OrderBook(time_t timestamp);
        ~OrderBook();

        BestBidOffer get_curr_bbo() const;

        int64_t get_best_bid() const;
        int64_t get_best_ask() const;
        uint64_t get_best_bid_quantity() const;
        uint64_t get_best_ask_quantity() const;

        bool handle_add_order(uint64_t id, const shared_ptr<Order>& order);
        bool handle_modify_order(uint64_t id, const ModifyFields * mf);

        bool handle_cancel_order(uint64_t id);
        bool handle_reduce_size(uint64_t id, uint32_t size);
        ExecutedOrder handle_order_executed(uint64_t id, const OrderExecuted * oex);

        bool contains(uint64_t order_id) const;
        shared_ptr<Order> find_order(uint64_t order_id) const;
        time_t get_start_time() const;
        uint64_t get_current_state_timestamp() const;
        bool set_current_state_timestamp(uint64_t ts);

        ExchangeStatus get_exchange_status() const;
        bool set_exchange_status(ExchangeStatus s);

        // L3 Functionality: Market By Order!
        BidQueue bids;
        AskQueue asks;

        // L2 Functionality: price level and quantity
        std::map<int64_t, uint64_t, std::greater<int64_t>> bid_quantity;
        std::map<int64_t, uint64_t, std::less<int64_t>> ask_quantity;
        // Persist pointers in local orderbook store
        std::unordered_map<uint64_t, std::shared_ptr<Order>> local_order_store;

    private:
        time_t start_time;
        uint64_t current_timestamp;
        ExchangeStatus currentTradingStatus {ExchangeStatus::S};
};



#endif //ORDER_BOOK_HPP
