
#include "include/order_book.hpp"

#include <algorithm>
#include <cassert>

#include <chrono>

using namespace std;

OrderBook::OrderBook() {
    // Inaccurate default time.
    start_time = std::time(nullptr);
    current_timestamp = 0;
    currentTradingStatus = ExchangeStatus::S;
}

OrderBook::OrderBook(time_t timestamp) {
    start_time = timestamp;
    current_timestamp = 0;
    currentTradingStatus = ExchangeStatus::S;
}

OrderBook::~OrderBook() {
    bids.clear();
    asks.clear();
    bid_quantity.clear();
    ask_quantity.clear();
    local_order_store.clear();
}



BestBidOffer OrderBook::get_curr_bbo() const
{
    const int64_t best_bid_price = this->get_best_bid();
    const uint32_t best_bid_amount = this->get_best_bid_quantity();
    const int64_t best_ask_price = this->get_best_ask();
    const uint32_t best_ask_amount = this->get_best_ask_quantity();
    return BestBidOffer{best_bid_price, best_bid_amount, best_ask_price, best_ask_amount};
}

int64_t OrderBook::get_best_bid() const {
    if (bids.empty()) {
        //cerr << "cannot fetch best bid price: EMPTY" << endl;
        return 0;
        //return INT64_MAX;
    }
    return bids.begin()->first;
}

int64_t OrderBook::get_best_ask() const {
    if (asks.empty()) {
        //cerr << "cannot fetch best ask price: EMPTY" << endl;
        return 0;
        //return INT64_MIN;
    }
    return asks.begin()->first;
}

uint64_t OrderBook::get_best_bid_quantity() const {
    if (bid_quantity.empty()) {
        //cerr << "cannot fetch best bid quantity: EMPTY" << endl;
        return 0;
    }
    return bid_quantity.begin()->second;
}

uint64_t OrderBook::get_best_ask_quantity() const {
    if (ask_quantity.empty()) {
        //cerr << "cannot fetch best ask quantity: EMPTY" << endl;
        return 0;
    }
    return ask_quantity.begin()->second;
}

bool OrderBook::handle_add_order(uint64_t id, const shared_ptr<Order>& order) {
    if (this->local_order_store.contains(id))
    {
        //cerr << "order already exists ID = "<< id << endl;
        return false;
    }

    if (order->getQuantity() == 0)
    {
        //cerr << "order with 0 quantity cannot be added ID:" << id << endl;
        return false;
    }

    this->local_order_store[id] = order;
    int64_t ordPrice = order->getPrice();

    if (order->getSide() == Side::Buy) {
        // USE operator[] to create a new entry for price if it doesn't exist, push orderptr
        // Use push_back usually, emplace_back is used to pass arguments (construct a new object)
        this->bids[ordPrice].emplace_back(order);
        // Update L2 quantity
        this->bid_quantity[ordPrice] += order->getQuantity();
    } else {
        // Add to asks
        this->asks[ordPrice].emplace_back(order);
        // Update L2 quantity
        this->ask_quantity[ordPrice] += order->getQuantity();
    }
    return true;
}


bool OrderBook::handle_cancel_order(uint64_t id) {

    if (!this->local_order_store.contains(id)) {
        //cerr << "order does not exist ID =" << id << endl;
        return false;
    }
    auto order = this->local_order_store[id];
    auto ordPrice = order->getPrice();

    if (order->getSide() == Side::Buy) {
        auto elem_to_erase = std::find_if(bids[ordPrice].begin(), bids[ordPrice].end(),
                        [&order](const auto& curbid){ return curbid->get_id() == order->get_id();});
        if (elem_to_erase != bids[ordPrice].end())
        {
            bids[ordPrice].erase(elem_to_erase);
        } else
        {
            //cerr << "BIDS order does not exist ID = " << id << endl;

        }
        //bids[ordPrice].erase(elem_to_erase);
        // L3 Update
        if (bids[ordPrice].empty())
        {
            bids.erase(ordPrice);
        }
        // L2 Update
        bid_quantity[ordPrice] -= order->getQuantity();
        if (bid_quantity[ordPrice] == 0) {
            bid_quantity.erase(ordPrice);
        }
    } else {
        auto elem_to_erase = std::find_if(asks[ordPrice].begin(), asks[ordPrice].end(),
                        [&order](const auto& ask){ return ask->get_id() == order->get_id();});

        if (elem_to_erase != asks[ordPrice].end())
        {
            asks[ordPrice].erase(elem_to_erase);
        } else
        {
            //cerr << "ASKS order does not exist ID = " << id << endl;

        }
        // asks[ordPrice].erase(elem_to_erase);


        // L3 Update
        if (asks[ordPrice].empty())
        {
            asks.erase(ordPrice);
        }
        // L2 Udpate
        ask_quantity[ordPrice] -= order->getQuantity();
        if (ask_quantity[ordPrice] == 0) {
            ask_quantity.erase(ordPrice);
        }
    }

    local_order_store.erase(id);
    return true;
}


bool OrderBook::handle_reduce_size(uint64_t id, uint32_t cxl_qty)
{
    if (this->local_order_store.contains(id))
    {
        auto order = this->local_order_store[id];
        try
        {
            int64_t ordPrice = order->getPrice();
            bool success = order->reduceQuantity(cxl_qty);
            if (success)
            {
                if (order->getSide() == Side::Buy)
                {
                    this->bid_quantity[ordPrice] -= cxl_qty;
                } else
                {
                    this->ask_quantity[ordPrice] -= cxl_qty;
                }

            } else
            {
                //std::ostringstream oss;
                //oss << "error handle_reduce_size: negative size suggested with this operation for order ID = "<< id << "not found";
                //cerr << oss.str() << endl;
            }
            return success;
        } catch (const std::logic_error& e)
        {
            throw std::logic_error(e.what());
        }
    }
    else
    {
        //throw std::invalid_argument(std::format("The order with ID {} does not exist in the orderbook", id));
    }
}


bool OrderBook::handle_modify_order(uint64_t order_id, const ModifyFields * mf)
{

    if (this->local_order_store.contains(order_id))
    {
        // Rewritten to preserve order posn in queue
        auto old_order = local_order_store[order_id];

        if (mf->new_price == old_order->getPrice() && mf->new_amt < old_order->getQuantity())
        {
            // Process the case where this modify operation came in as a reduce
            uint32_t new_qty = old_order->getQuantity() - mf->new_amt;
            const bool res_1 = handle_reduce_size(order_id, new_qty);
            //cerr << "Order updated in place, kept priority" << order_id << endl;
            return res_1;
        } else
        {
            // Change in price or NO-OP results in loss of priority
            // TODO: make copy of order to ensure nothing gets changed

            const bool res_1 = handle_cancel_order(order_id);
            // SET NEW PRICE/QTY AFTER CANCEL!
            int64_t new_price = mf->new_price;
            uint32_t new_quantity = mf->new_amt;
            old_order->setPrice(new_price);
            old_order->setQuantity(new_quantity);
            const bool res_2 = handle_add_order(old_order->get_id(), old_order);
            //cerr << "Order reinserted and lost priority due to modify" << order_id << endl;
            return res_1 && res_2;
        }
    }
    //cerr << "order does not exist ID" << order_id << endl;

    return false;
}

bool OrderBook::contains(uint64_t order_id) const
{
    return this->local_order_store.contains(order_id);
}

ExecutedOrder OrderBook::handle_order_executed(uint64_t order_id, const OrderExecuted * oex)
{

    if (this->local_order_store.contains(order_id))
    {
        auto order = this->local_order_store[order_id];

        uint16_t exec_qty = oex->ExecutedQuantity;
        bool complete_fill = false;
        if (order->getQuantity() == exec_qty)
        {
            complete_fill = true;
        }

        bool correctness = order->reduceQuantity(exec_qty);
        const int64_t ex_price = order->getPrice();
        auto ex_ord = ExecutedOrder(*order, ex_price, exec_qty);

        if (!correctness)
        {
            //cerr << "order with ID = {} cannot be executed: Insufficient Quantity." << endl;
        }

        if (complete_fill)
        {
            // HANDLE LIKE CANCEL IF A COMPLETE FILL
            if (order->getSide() == Side::Buy)
            {
                if (order_id != this->bids[ex_price].front()->get_id())
                {
                    // This is a data integrity error: order book state fatally wrong
                    //throw std::logic_error(std::format("Order id {} is not first in bid queue", order->get_id()));
                    throw std::logic_error("ORDER BOOK ERROR STATE");
                }
                this->bids[ex_price].pop_front();
                this->bid_quantity[ex_price] -= exec_qty;
                // L3 Update
                if (bids[ex_price].empty()) {bids.erase(ex_price);}
                // L2 Udpate
                if (bid_quantity[ex_price] == 0) {bid_quantity.erase(ex_price);}
            }
            else
            {
                if (order_id != this->asks[ex_price].front()->get_id())
                {
                    // This is a data integrity error: order book state fatally wrong
                    //throw std::logic_error(std::format("Order id {} is not first in bid queue", order->get_id()));
                    throw std::logic_error("ORDER BOOK ERROR STATE");
                }
                this->asks[ex_price].pop_front();
                this->ask_quantity[ex_price] -= exec_qty;
                // L3 Update
                if (asks[ex_price].empty()) {asks.erase(ex_price);}
                // L2 Udpate
                if (ask_quantity[ex_price] == 0) {ask_quantity.erase(ex_price);}
            }
            // Remove order from local order store
            this->local_order_store.erase(order_id);
        } else
        {
            // Handle Partial fill at L2 level solely
            if (order->getSide() == Side::Buy)
            {
                this->bid_quantity[order->getPrice()] -= exec_qty;
            } else
            {
                this->ask_quantity[order->getPrice()] -= exec_qty;
            }
        }
        return ex_ord;


    } else
    {
        // TODO: modify for when we execute the orders in place
        throw std::logic_error("ERROR! Order Executed Is Invalid");
    }

}

OrderPtr OrderBook::find_order(const uint64_t order_id) const
{
    if (this->local_order_store.contains(order_id))
        return this->local_order_store.at(order_id);

    else
    {
        return nullptr;
    }
}

time_t OrderBook::get_start_time() const
{
    return this->start_time;
}

uint64_t OrderBook::get_current_state_timestamp() const
{
    return this->current_timestamp;
}

bool OrderBook::set_current_state_timestamp(uint64_t ts)
{
    this->current_timestamp = ts;
    return true;
}

ExchangeStatus OrderBook::get_exchange_status() const
{
    return this->currentTradingStatus;
}

bool OrderBook::set_exchange_status(const ExchangeStatus s)
{
    this->currentTradingStatus = s;
    return true;
}




