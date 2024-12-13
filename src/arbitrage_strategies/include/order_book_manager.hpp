#pragma once

#ifndef ORDER_BOOK_MANAGER_HPP
#define ORDER_BOOK_MANAGER_HPP
#include <unordered_map>
#include <memory>
//#include <mutex>

#include "order_book.hpp"
#include "order_structures.hpp"
#include "cfepitch.h"

using namespace std;


class OrderBookManager {
    public:
        OrderBookManager();
        OrderBookManager(time_t start_time);
        ~OrderBookManager();

        bool contains_order(uint64_t order_id);
        Order get_order(uint64_t order_id);
        OrderBook find_orderbook(uint64_t order_id) const;
        OrderBook find_orderbook_by_ticker(const string &order_book_name);
        bool contains_orderbook(const string& ticker) const;
        const OrderBook& operator[](string& ticker) const;

        // Add current_timestamp in ns to this class start_time to get new time_t
        bool add_orderbook(const string& ticker);
        bool remove_orderbook(const string& ticker);
        static bool check_bbo_changed(const BestBidOffer& oldBBO, const BestBidOffer& newBBO);

        // 5 Relevant CBOE operations to BBO
        // Returns pair: <WHETHER BBO CHANGED, CURRENT BBO LEVEL>
        pair<bool, BestBidOffer> handle_CBOE_add_order(uint64_t order_id, string& symbol, int64_t price, uint32_t quantity, Side side);
        pair<bool, BestBidOffer> handle_CBOE_cancel_order(uint64_t order_id);
        pair<bool, BestBidOffer> handle_CBOE_modify_order(uint64_t order_id, int64_t new_price, uint32_t new_quantity);
        pair<bool, BestBidOffer> handle_CBOE_reduce_size(uint64_t order_id, uint32_t reduce_size);
        pair<bool, BestBidOffer> handle_CBOE_order_executed(uint64_t order_id, OrderExecuted& order_executed);

        time_t get_start_time() const;
        bool set_start_time(time_t ts);
        uint64_t get_current_state_timestamp() const;
        bool set_current_state_timestamp(uint64_t ts);
        ExchangeStatus get_ticker_trading_status(const string& ticker);
        bool set_ticker_trading_status(const string& ticker, ExchangeStatus status);

    protected:
        std::unordered_map<string, OrderBook> orderBooks;
        std::unordered_map<string, BestBidOffer> orderBooksCurrBBO;
        std::unordered_map<uint64_t, shared_ptr<Order>> idToOrder;

        vector<ExecutedOrder> executedOrders;

        time_t start_time;
        uint64_t current_timestamp;

        // Future Fields: potential multithreading?
        //std::mutex orderBooksMutex;
};

class OrderStore
{
public:

    OrderStore();
    ~OrderStore();
    bool add_order(shared_ptr<Order>);
    bool remove_order(uint64_t order_id);
    shared_ptr<Order> get_order(uint64_t order_id);


private:
    std::unordered_map<uint64_t, shared_ptr<Order>> idToOrder;

};



#endif //ORDER_BOOK_MANAGER_HPP
