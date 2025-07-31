

#ifndef ORDER_STRUCTURES_HPP
#define ORDER_STRUCTURES_HPP
#include <iostream>
#include <ostream>
#include <cstdint>
#include <string>
#include <memory>
#include <sstream>
#include <algorithm>
#include <list>

using namespace std;



struct ModifyFields
{
    int64_t new_price;
    uint32_t new_amt;
};

struct BestBidOffer
{
    int64_t best_bid_price;
    uint32_t best_bid_amount;
    int64_t best_ask_price;
    uint32_t best_ask_amount;

    [[nodiscard]] long double getTheo() const {
        long double bid_p = static_cast<long double>(best_bid_price);
        long double bid_q = static_cast<long double>(best_bid_amount);
        long double ask_p = static_cast<long double>(best_ask_price);
        long double ask_q = static_cast<long double>(best_ask_amount);

        if (bid_q + ask_q == 0) {
            // cerr<< div by 0 error<<endl;
            return 0.0L;
        }

        return ((bid_p * ask_q) + (ask_p * bid_q)) / (bid_q + ask_q);
    }

};



struct BBO_CSV_Entry
{
    std::string srcTime;
    uint64_t pktSeqNum;
    uint64_t msgSeqNum;
    char msgType;
    std::string Symbol;
    uint32_t bidQty;
    int64_t bidPx;
    uint32_t askQty;
    int64_t askPx;
    char tradingStatus;
};






enum class Side
{
    Buy,
    Sell,
};

enum class CBOEOperation {
    AddOrder,
    ReduceSize,
    ModifyOrder,
    CancelOrder,
    OrderExecuted
};

enum class ExchangeStatus {
    Q, // Queueing
    T, // Trading
    S // Suspended
};


struct L2LevelRecord
{
    uint32_t qty;
    uint32_t numOrders;
    Side side;

    L2LevelRecord(uint32_t q, uint32_t nord, Side s) :  qty(q), numOrders(nord), side(s) {}

};



struct L2RecordEntry
{
    std::string srcTime; // AT PRINT
    uint64_t pktSeqNum; // AT PRINT
    uint64_t msgSeqNum; // AT PRINT
    char msgType; // AT PRINT
    std::string Symbol; // AT BUILD
    char tradingStatus; // AT PRINT
    double ticksize; // AT BUILD

    // Best Bid/Offer
    int64_t best_bid_price; // AT BUILD
    int64_t best_ask_price; // AT BUILD
    
    // Sort the bids descending, asks ascending, index is amount below/above BBO
    // index 1 is 1 tick below/above BBO
    // index 0 is the BBO itself
    std::vector<L2LevelRecord> bids; // Sorted descending by price, AT BUILD
    std::vector<L2LevelRecord> asks; // Sorted ascending by price, AT BUILD

};



class Order {
public:
    Order(uint64_t orderId, Side side, uint32_t quantity, const std::string& symbol, int64_t price)
        : id(orderId), side(side), init_size(quantity), quantity(quantity), symbol(symbol), price(price), virtual_quantity(quantity) {}

    uint64_t get_id() const { return id; }
    Side getSide() const { return side; }
    uint32_t getQuantity() const { return quantity; }
    uint32_t getVirtualQuantity() const { return virtual_quantity; }
    uint32_t getInitialSize() const { return init_size; }
    const std::string& getSymbol() const { return symbol; }
    int64_t getPrice() const { return price; }

    Order(const Order& other)
        : id(other.id),
          side(other.side),
          init_size(other.init_size),
          quantity(other.quantity),
          symbol(other.symbol),
          price(other.price)
    {
    }

    Order(Order&& other) noexcept
        : id(other.id),
          side(other.side),
          init_size(other.init_size),
          quantity(other.quantity),
          symbol(std::move(other.symbol)),
          price(other.price)
    {
    }

    Order& operator=(const Order& other)
    {
        if (this == &other)
            return *this;
        id = other.id;
        side = other.side;
        init_size = other.init_size;
        quantity = other.quantity;
        symbol = other.symbol;
        price = other.price;
        return *this;
    }

    Order& operator=(Order&& other) noexcept
    {
        if (this == &other)
            return *this;
        id = other.id;
        side = other.side;
        init_size = other.init_size;
        quantity = other.quantity;
        symbol = std::move(other.symbol);
        price = other.price;
        return *this;
    }

    bool setQuantity(uint32_t qty)
    {

        if (qty > UINT16_MAX)
        {
            //std::cerr << "ERROR SET QTY: Above max for CBOE qty limit" << std::endl;
        }
        this->quantity = qty;
        return true;
    };


    bool reduceQuantity(uint32_t red_quantity)
    {
        if (this->quantity < red_quantity)
        {

            //std::ostringstream oss;
            //oss << "ERROR REDUCE QTY: Cannot have Negative Quantity for order ID = " << this->id;
            //std::cerr << oss.str() << std::endl;
            // throw std::logic_error(err);
            return false;
        }
        this->quantity -= red_quantity;

        uint32_t reduceable_quantity = this->quantity;
        // Bug fixed: must cap virtual quantity so it does not overflow.
        this->virtual_quantity -= std::min(reduceable_quantity, red_quantity);
        return true;
    }


    bool reduceVirtualQuantity(uint32_t red_quantity)
    {
        if (this->virtual_quantity < red_quantity)
        {
            return false;
        }
        this->virtual_quantity -= red_quantity;
        return true;
    }

    void setPrice(int64_t price)
    {
        this->price = price;
    }




protected:
    uint64_t id;
    Side side;
    uint32_t init_size;
    uint32_t quantity;
    std::string symbol;
    int64_t price;
    uint32_t virtual_quantity; // Arbed quantity

public:
    virtual ~Order() = default;
};

class ExecutedOrder : public Order {
public:
    ExecutedOrder(uint64_t orderId, Side side, uint32_t initialQuantity, const std::string& symbol,
                  int64_t initialPrice, uint32_t executedQuantity, int64_t executedPrice)
        : Order(orderId, side, initialQuantity, symbol, initialPrice),
          executedQuantity(executedQuantity), executedPrice(executedPrice) {}

    ExecutedOrder(const Order& order, uint32_t executedQuantity, int64_t executedPrice)
        : Order(order.get_id(), order.getSide(), order.getQuantity(), order.getSymbol(), order.getPrice()),
          executedQuantity(executedQuantity), executedPrice(executedPrice) {}

    ExecutedOrder(const ExecutedOrder& other)
        : Order(other),
          executedQuantity(other.executedQuantity),
          executedPrice(other.executedPrice)
    {
    }

    ExecutedOrder(ExecutedOrder&& other) noexcept
        : Order(std::move(other)),
          executedQuantity(other.executedQuantity),
          executedPrice(other.executedPrice)
    {
    }

    ExecutedOrder& operator=(const ExecutedOrder& other)
    {
        if (this == &other)
            return *this;
        Order::operator =(other);
        executedQuantity = other.executedQuantity;
        executedPrice = other.executedPrice;
        return *this;
    }

    ExecutedOrder& operator=(ExecutedOrder&& other) noexcept
    {
        if (this == &other)
            return *this;
        Order::operator =(std::move(other));
        executedQuantity = other.executedQuantity;
        executedPrice = other.executedPrice;
        return *this;
    }

    uint32_t getExecutedQuantity() const { return executedQuantity; }
    int64_t getExecutedPrice() const { return executedPrice; }

    void setExecutedQuantity(uint32_t exQuantity) { this->executedQuantity = exQuantity; }
    void setExecutedPrice(int64_t exPrice) { this->executedQuantity = exPrice; }


private:
    uint32_t executedQuantity;
    int64_t executedPrice;
};

// class VirtualOrder : public Order {
// public:
//     VirtualOrder(uint64_t orderId, Side side, uint32_t quantity, const std::string& symbol, int64_t price)
//         : Order(orderId, side, quantity, symbol, price) {}
//
//     // Copy constructor
//     VirtualOrder(const VirtualOrder& other) = default;
//
//     // Move constructor
//     VirtualOrder(VirtualOrder&& other) noexcept = default;
//
//     // Copy assignment operator
//     VirtualOrder& operator=(const VirtualOrder& other) {
//         if (this == &other)
//             return *this;
//         Order::operator=(other);
//         return *this;
//     }
//
//     // Move assignment operator
//     VirtualOrder& operator=(VirtualOrder&& other) noexcept {
//         if (this == &other)
//             return *this;
//         Order::operator=(std::move(other));
//         return *this;
//     }
//
//     // Constructor from Order
//     explicit VirtualOrder(const Order& order) : Order(order) {}
//
//     // Constructor from Order rvalue
//     explicit VirtualOrder(Order&& order) noexcept : Order(std::move(order)) {}
//
//     ~VirtualOrder() override = default;
// };

struct PriceLevel {
    int64_t price;
    vector<shared_ptr<Order>> available_orders;

    [[nodiscard]] uint32_t getAvailableQuantity() const {
        uint32_t total = 0;
        for (const auto& order : available_orders) {
            total += order->getVirtualQuantity();
        }
        return total;
    }
};

//using OrderPtr = std::shared_ptr<Order>;
//using OrderQueue = std::list<OrderPtr>;


#endif //ORDER_STRUCTURES_HPP
