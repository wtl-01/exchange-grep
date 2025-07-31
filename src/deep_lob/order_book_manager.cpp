
#include "include/order_book_manager.hpp"

OrderBookManager::OrderBookManager() {
    this->start_time = std::time(nullptr);
    current_timestamp = 0;
}

OrderBookManager::OrderBookManager(time_t start_time) {
    this->start_time = start_time;
    current_timestamp = 0;
}



OrderBookManager::~OrderBookManager() {
    orderBooks.clear();
    orderBooksCurrBBO.clear();
    idToOrder.clear();
    executedOrders.clear();
}

bool OrderBookManager::contains_order(uint64_t order_id) {
    return idToOrder.contains(order_id);
}

Order OrderBookManager::get_order(uint64_t order_id) {
    if (!contains_order(order_id)) {

        std::ostringstream oss;
        oss << "Order with ID = "<< order_id << "not found" ;
        throw std::out_of_range(oss.str());
    }
    return *idToOrder[order_id];
}

OrderBook OrderBookManager::find_orderbook(uint64_t order_id) const {
    if (!idToOrder.contains(order_id)) {
        std::ostringstream oss;
        oss << "Order with ID = "<< order_id << "not found" ;
        throw std::out_of_range(oss.str());
    }

    const auto& order = idToOrder.at(order_id);
    const auto& symbol = order->getSymbol();

    if (!orderBooks.contains(symbol)) {
        std::ostringstream oss;
        oss << "Order with ID = "<< order_id << "not found" ;
        throw std::out_of_range(oss.str());
    }

    return orderBooks.at(symbol);
}

OrderBook OrderBookManager::find_orderbook_by_ticker(const string& order_book_name) {
    if (!orderBooks.contains(order_book_name)) {
        std::ostringstream oss;
        oss << "Order Book with Ticker = "<< order_book_name << "not found" ;
        throw std::out_of_range(oss.str());
    }
    return orderBooks[order_book_name];
}

bool OrderBookManager::contains_orderbook(const string& ticker) const
{
    return orderBooks.contains(ticker);
}

const OrderBook& OrderBookManager::operator[](string& ticker) const {
    if (!orderBooks.contains(ticker)) {
        std::ostringstream oss;
        oss << "Order Book with Ticker = "<< ticker << "not found" ;
        throw std::out_of_range(oss.str());
    }
    return orderBooks.at(ticker);
}

bool OrderBookManager::add_orderbook(const string& ticker) {
    if (orderBooks.contains(ticker)) {
        return false;
    }

    orderBooks.emplace(ticker, OrderBook(start_time));
    orderBooksCurrBBO.emplace(ticker, BestBidOffer{INT64_MAX, 0, INT64_MIN, 0});
    return true;
}

bool OrderBookManager::remove_orderbook(const string& ticker) {
    if (!orderBooks.contains(ticker)) {
        return false;
    }

    // TODO: NO NEED TO REMOVE TICKERS BELONGING TO ORDERBOOK

    orderBooks.erase(ticker);
    orderBooksCurrBBO.erase(ticker);
    return true;
}

bool OrderBookManager::check_bbo_changed(const BestBidOffer& oldBBO, const BestBidOffer& newBBO)
{
    return (oldBBO.best_bid_price != newBBO.best_bid_price ||
        oldBBO.best_bid_amount != newBBO.best_bid_amount ||
        oldBBO.best_ask_price != newBBO.best_ask_price ||
        oldBBO.best_ask_amount != newBBO.best_ask_amount);
}

pair<bool, BestBidOffer> OrderBookManager::handle_CBOE_add_order(
    uint64_t order_id, string& symbol, int64_t price, uint32_t quantity, Side side) {

    if (!orderBooks.contains(symbol)) {
        std::ostringstream oss;
        oss << "Order Book with Ticker = "<< symbol << "not found";
        throw std::out_of_range(oss.str());
    }

    auto& orderBook = orderBooks[symbol];
    BestBidOffer oldBBO = orderBooksCurrBBO[symbol];

    // Create and store the order
    auto order = std::make_shared<Order>(order_id, side, quantity, symbol, price);
    idToOrder[order_id] = order;

    // Add to order book
    bool success = orderBook.handle_add_order(order_id, order);
    if (!success) {
        idToOrder.erase(order_id);
        return {false, oldBBO};
    }

    // Update BBO
    BestBidOffer newBBO = orderBook.get_curr_bbo();
    orderBooksCurrBBO[symbol] = newBBO;


    bool bboChanged = check_bbo_changed(oldBBO, newBBO);

    return {bboChanged, newBBO};
}

pair<bool, BestBidOffer> OrderBookManager::handle_CBOE_cancel_order(uint64_t order_id) {
    if (!idToOrder.contains(order_id)) {
        std::ostringstream oss;
        oss << "Order with ID = "<< order_id << " not found" ;
        throw std::out_of_range(oss.str());
    }

    auto order = idToOrder[order_id];
    auto& symbol = order->getSymbol();
    auto& orderBook = orderBooks[symbol];

    BestBidOffer oldBBO = orderBooksCurrBBO[symbol];

    bool success = orderBook.handle_cancel_order(order_id);
    if (!success) {
        return {false, oldBBO};
    }

    idToOrder.erase(order_id);

    //cerr << "[CANCEL ORDER] Order with ID = " << order_id << " Erased" << endl;


    BestBidOffer newBBO = orderBook.get_curr_bbo();
    orderBooksCurrBBO[symbol] = newBBO;

    bool bboChanged = check_bbo_changed(oldBBO, newBBO);
    return {bboChanged, newBBO};
}

pair<bool, BestBidOffer> OrderBookManager::handle_CBOE_modify_order(
    uint64_t order_id, int64_t new_price, uint32_t new_quantity) {

    if (!idToOrder.contains(order_id)) {
        std::ostringstream oss;
        oss << "Order with ID = "<< order_id << " not found" ;
        throw std::out_of_range(oss.str());
    }

    auto order = idToOrder[order_id];
    auto& symbol = order->getSymbol();
    auto& orderBook = orderBooks[symbol];

    BestBidOffer oldBBO = orderBooksCurrBBO[symbol];

    ModifyFields mf{new_price, new_quantity};
    bool success = orderBook.handle_modify_order(order_id, &mf);
    if (!success) {
        return {false, oldBBO};
    }

    BestBidOffer newBBO = orderBook.get_curr_bbo();
    orderBooksCurrBBO[symbol] = newBBO;

    if (new_quantity == 0)
    {
        idToOrder.erase(order_id);
        //cerr << "[MODIFY ORDER] Order with ID = "<< order_id << "Modified" << endl;

    }

    bool bboChanged = check_bbo_changed(oldBBO, newBBO);
    return {bboChanged, newBBO};
}

pair<bool, BestBidOffer> OrderBookManager::handle_CBOE_reduce_size(
    uint64_t order_id, uint32_t reduce_size) {

    if (!idToOrder.contains(order_id)) {
        std::ostringstream oss;
        oss << "Order with ID = "<< order_id << "not found" ;
        throw std::out_of_range(oss.str());
    }

    auto order = idToOrder[order_id];

    auto& symbol = order->getSymbol();
    auto& orderBook = orderBooks[symbol];

    BestBidOffer oldBBO = orderBooksCurrBBO[symbol];

    bool success = orderBook.handle_reduce_size(order_id, reduce_size);
    if (!success) {
        return {false, oldBBO};
    }


    BestBidOffer newBBO = orderBook.get_curr_bbo();
    orderBooksCurrBBO[symbol] = newBBO;

    bool bboChanged = check_bbo_changed(oldBBO, newBBO);

    return {bboChanged, newBBO};
}

pair<bool, BestBidOffer> OrderBookManager::handle_CBOE_order_executed(
    uint64_t order_id, OrderExecuted& order_executed) {

    if (!idToOrder.contains(order_id)) {
        std::ostringstream oss;
        oss << "Order with ID = "<< order_id << "not found" ;
        throw std::out_of_range(oss.str());
    }

    auto order = idToOrder[order_id];
    auto& symbol = order->getSymbol();
    auto& orderBook = orderBooks[symbol];

    BestBidOffer oldBBO = orderBooksCurrBBO[symbol];

    ExecutedOrder executed_order = orderBook.handle_order_executed(order_id, &order_executed);
    executedOrders.reserve(1);
    executedOrders.push_back(executed_order);

    // TODO: Check assumption: orderbook should have reduced qty to 0
    if (order->getQuantity() == 0) {
        idToOrder.erase(order_id);
        //cerr << "[ORDER EXECUTED] Order with ID = "<<order_id << "Executed" << endl;
    }

    BestBidOffer newBBO = orderBook.get_curr_bbo();
    orderBooksCurrBBO[symbol] = newBBO;

    bool bboChanged = check_bbo_changed(oldBBO, newBBO);
    return {bboChanged, newBBO};
}

time_t OrderBookManager::get_start_time() const {
    return start_time;
}

bool OrderBookManager::set_start_time(time_t ts)
{
    this->start_time = ts;
    return true;
}

uint64_t OrderBookManager::get_current_state_timestamp() const {
    return current_timestamp;
}

bool OrderBookManager::set_current_state_timestamp(uint64_t ts) {
    if (ts < current_timestamp) {
        //std::cerr << "ERROR: OrderBookManager::set_current_state_timestamp() called with new timestamp before current timestamp" << std::endl;
        return false;
    }
    current_timestamp = ts;
    // Update timestamp for all order books
    for (auto& [symbol, orderBook] : orderBooks) {
        bool successful_curr = orderBook.set_current_state_timestamp(ts);
        if (!successful_curr)
        {
            //cerr << "ERROR: OrderBookManager::set_current_state_timestamp() failed for orderbook ticker = " << symbol << std::endl;
            return false;
        }
    }
    return true;
}

ExchangeStatus OrderBookManager::get_ticker_trading_status(const string& ticker)
{
    auto& orderBook = orderBooks[ticker];
    return orderBook.get_exchange_status();
}

bool OrderBookManager::set_ticker_trading_status(const string& ticker, ExchangeStatus status)
{
    auto& orderBook = orderBooks[ticker];
    return orderBook.set_exchange_status(status);
}

OrderStore::OrderStore() = default;

OrderStore::~OrderStore() = default;

bool OrderStore::add_order(shared_ptr<Order> order) {
    if (!order) {
        return false;
    }

    uint64_t order_id = order->get_id();
    if (idToOrder.contains(order_id)) {
        return false;
    }

    idToOrder[order_id] = order;
    return true;
}

bool OrderStore::remove_order(uint64_t order_id) {
    return idToOrder.erase(order_id) > 0;
}

shared_ptr<Order> OrderStore::get_order(uint64_t order_id)
{
    auto it = idToOrder.find(order_id);
    if (it == idToOrder.end())
    {
        return nullptr;
    }
    return it->second;
}

// L2RecordEntry get_L2_record(const string& machine_ticker)
L2RecordEntry OrderBookManager::get_L2_record(const string& machine_ticker, unordered_map<string, unordered_map<string, string>>& srd)
{
    if (!orderBooks.contains(machine_ticker)) {
        std::ostringstream oss;
        oss << "Order Book with Exchange Ticker = "<< machine_ticker << "not found" ;
        throw std::out_of_range(oss.str());
    }

    auto& orderBook = orderBooks[machine_ticker];

    L2RecordEntry l2_record;

    // get the tick size and human readable ticker with symbol_to_readable_dict.hpp
    if (srd.contains(machine_ticker)) {
        // for human readable ticker, extract "name" field
        auto& info_map = srd[machine_ticker];
        if (info_map.contains("name")) {
            l2_record.Symbol = info_map["name"];
        } else {
            l2_record.Symbol = machine_ticker; // fallback to machine ticker
        }

        // for tick size, extract "tick" field
        if (info_map.contains("tick")) {
            try {
                l2_record.ticksize = std::stod(info_map["tick"]);
            } catch (const std::exception e) {
                // if conversion fails, set to default
                // output warning to cerr
                std::cerr << "Warning: Failed to convert tick size for ticker " << machine_ticker << ". Setting to default 0.01." << std::endl;
                l2_record.ticksize = 0.01; // default tick size
            }
        } else {
            // error: tick size not found, set to default
            cerr << "Warning: Tick size not found for ticker " << machine_ticker << ". Setting to default 0.01." << std::endl;
            l2_record.ticksize = 0.01; // default tick size
        }
    }
    else {
        std::ostringstream oss;
        oss << "Ticker " << machine_ticker << " not found in symbol_to_readable_dict" ;
        throw std::out_of_range(oss.str());
    }

    // Get top 10 bids
    // l2_record.tradingStatus = static_cast<char>(orderBook.get_exchange_status());
    l2_record.best_bid_price = orderBook.get_best_bid();
    l2_record.best_ask_price = orderBook.get_best_ask();

    // Populate bids
    auto bids = orderBook.bid_quantity;
    auto bidqueue = orderBook.bids;
    if (!bids.empty()) {
        int count = 0;
        for (const auto& [price, qty] : bids) {
            if (count >= 10) break;
            uint32_t numOrders = 0;
            if (bidqueue.contains(price)) {
                numOrders = static_cast<uint32_t>(bidqueue[price].size());
            }
            l2_record.bids.emplace_back(L2LevelRecord(static_cast<uint32_t>(qty), numOrders, Side::Buy));
            count++;
        }
    }

    // Populate asks
    auto asks = orderBook.ask_quantity;
    auto askqueue = orderBook.asks;
    if (!asks.empty()) {
        int count = 0;
        for (const auto& [price, qty] : asks) {
            if (count >= 10) break;
            uint32_t numOrders = 0;
            if (askqueue.contains(price)) {
                numOrders = static_cast<uint32_t>(askqueue[price].size());
            }
            l2_record.asks.emplace_back(L2LevelRecord(static_cast<uint32_t>(qty), numOrders, Side::Sell));
            count++;
        }
    }


    return l2_record;
}