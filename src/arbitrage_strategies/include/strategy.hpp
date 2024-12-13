

#ifndef STRATEGY_HPP
#define STRATEGY_HPP

#include <map>
#include <queue>
#include <memory>
#include <cstdint>
#include <ctime>
#include <list>
#include <unordered_map>
#include "order_structures.hpp"
#include "order_book.hpp"
#include "order_book_manager.hpp"
#include <set>
#include <map>

using namespace std;

enum StrategyType {
  MINI_FULL_ARB = 0, // 1 of full, -10 of mini
  TRIANGLE_ARB = 1, // 1 SPREAD, 1 SHORT LEG, -1 LONG LEG
  CUSTOM = 300
};

// TODO: This reflects a static portfolio for arbitrage
// NOTE: The primary strategy ticker is always at the first location
// Eg: if our strategy is Long -VXV4+VXZ4, Long VXV4, Short VXZ4
// then positions = [1, 1, -1]
// Then the spread is in index 0 with position 1
// Eg 2: Long VXV4, Short VXMV4: positions = [1, -10]
struct Portfolio {
    uint8_t num_legs;
    // Support up to 8 instruments per portfolio
    // Primary Strategy is always at index 0
    string hr_ticker[8];
    int32_t quantity[8];

    Portfolio multiplyQuantities(uint32_t multiplier) const {
        Portfolio newPortfolio;

        // Copy num_legs
        newPortfolio.num_legs = this->num_legs;

        // Copy and multiply each quantity, and copy the hr_ticker
        for (uint8_t i = 0; i < this->num_legs; ++i) {
            newPortfolio.hr_ticker[i] = this->hr_ticker[i];
            newPortfolio.quantity[i] = this->quantity[i] * multiplier;
        }

        return newPortfolio;
    }

    string toString() const {
        std::ostringstream oss;

        oss << "Portfolio (Legs: " << static_cast<int>(num_legs) << "): [";

        for (uint8_t i = 0; i < num_legs; ++i) {
            oss << "{Ticker: " << hr_ticker[i]
                << "; Quantity: " << quantity[i] << "}";

            if (i < num_legs - 1) {
                oss << "; ";
            }
        }

        oss << "]";
        return oss.str();
    }



    // Equality operator
    bool operator==(const Portfolio& other) const {
        if (num_legs != other.num_legs) return false;

        for (int i = 0; i < num_legs; i++) {
            if (hr_ticker[i] != other.hr_ticker[i] ||
                quantity[i] != other.quantity[i]) {
                return false;
                }
        }
        return true;
    }

    // Inequality operator
    bool operator!=(const Portfolio& other) const {
        return !(*this == other);
    }
};



class StrategyInstance {
	public:
		StrategyInstance(Portfolio method_port, uint64_t to) {
            this->strategy_method = method_port;
            this->time_offset = to;
            this->pnl = 0;
		};
        ~StrategyInstance() = default;
        StrategyInstance(const StrategyInstance &) = default;
        StrategyInstance(StrategyInstance &&) noexcept = default;


        bool setTimeOffset(uint64_t to) {
            this->time_offset = to;
            return true;
        };


        uint64_t getTimeOffset() {
            return this->time_offset;
        };

        Portfolio getInitialPortfolio() {
            return this->strategy_method;
        };

        bool setInitialPortfolio(Portfolio method) {
            this->strategy_method = method;
            return true;
        };

        Portfolio getFinalPortfolio() {
            return this->strategy_method.multiplyQuantities(this->size_of_strategy);
        };
        bool setFinalPortfolio(uint32_t size) {
            this->size_of_strategy = size;
            return true;
        };

        int64_t get_pnl() {
            return this->pnl;
        };
        bool set_pnl(int64_t pnl_set) {
            this->pnl = pnl_set;
            return true;
        };
//        int64_t calc_pnl() {
//            int64_t curr_pnl = 0;
//            auto it1 = this->price_at_holding.begin();
//            auto it2 = this->quantity_at_holding.begin();
//
//            while (it1 != this->price_at_holding.end() && it2 != this->quantity_at_holding.end()) {
//                pnl += (*it1) * (*it2);
//                ++it1;
//                ++it2;
//            }
//            return curr_pnl;
//        };

    Portfolio strategy_method;
    uint32_t size_of_strategy;
    list<uint64_t> order_id_used;
    //list<int64_t> price_at_holding;
    //list<uint32_t> quantity_at_holding;
    list<tuple<string, int64_t, uint32_t>> level_sizes;
    int64_t pnl;
    uint64_t time_offset;




};



class StrategyManager {
public:
    StrategyManager() = default;
    ~StrategyManager() = default;
    StrategyManager(const StrategyManager &) = default;
    StrategyManager(StrategyManager &&) noexcept = default;

    bool addStrategyInstance(StrategyInstance strat) {
        this->strategies.emplace_back(strat);
        return true;
    };
    list<StrategyInstance> getStrategyInstances() {
      return this->strategies;
    };

    bool containsStrategy(string hr_ticker) {
        return this->ticker_to_strat_map.contains(hr_ticker);
    };

    list<Portfolio> getTickerStrategies(string hr_ticker) {
        if (!this->ticker_to_strat_map.contains(hr_ticker) || this->ticker_to_strat_map[hr_ticker].size() == 0) {
            //std::ostringstream oss;
            //oss << "No strategies for ticker = "<< hr_ticker << "found at all." ;
            //throw std::out_of_range(oss.str());
            return list<Portfolio>();
        }
        return this->ticker_to_strat_map[hr_ticker];
    };

    bool setTickerStrategies(Portfolio pf) {
        for (const auto& existing_pf : ticker_to_strat_map[pf.hr_ticker[0]]) {
            if (existing_pf == pf) {
                //cerr << "Portfolio already exists in the strategy list." << endl;
                return false;
            }
        }

        for (const auto& hr_ticker : pf.hr_ticker) {
            ticker_to_strat_map[hr_ticker].emplace_back(pf);
        }

        return true;
    };

    // Returns a strategy instance given the strategy. Match the maximal arbitrage possible.
    static StrategyInstance matchStrategyOrders(OrderBookManager* obm, Portfolio strategy, unordered_map<string, string> rtedict) {

        string primary_ticker = strategy.hr_ticker[0];
        int64_t primary_position = strategy.quantity[0];
        //if (primary_position != 1) {
            //cerr << "Primary strategy must have notional size 1" << endl;
            //throw runtime_error("Strategy Invalid Primary Strategy not Size 1");
        //}

        vector<tuple<string, OrderBook, int64_t>> legs;
        for (int i = 0; i < strategy.num_legs; i++) {
            auto curr_hr_ticker = strategy.hr_ticker[i];
            auto curr_position = strategy.quantity[i];
            string curr_ticker = rtedict[curr_hr_ticker];
            try{
                OrderBook curr_ob = obm->find_orderbook_by_ticker(curr_ticker);

                if (curr_ob.get_exchange_status() != ExchangeStatus::T) {
                    throw out_of_range("Trading not currently active for this ticker.");
                }
                legs.emplace_back(curr_hr_ticker, curr_ob, curr_position);
                //cerr << "Strategy" << strategy.toString() << "with orderbook " << curr_ticker << " is valid." << endl;
            }
            catch (out_of_range e) {
                //cerr << "Strategy" << strategy.toString() << "with orderbook " << curr_ticker << " is invalid." << endl;
                throw out_of_range(e.what());
            }

        }
        // Then, match and execute maximal quantities at this price level
        // Keep executing until profit of the current strategy is at or below 0, then break.
        vector<vector<PriceLevel>> legLevels(strategy.num_legs);

        for (size_t i = 0; i < legs.size(); i++) {
            // Iterate through orderbooks
            const auto& [leg, ob, position] = legs[i];

            if (position > 0) {
                // Positive weight: Long. Look at Asks
                for (const auto& [price, orderQueue] : ob.asks) {
                    PriceLevel level{price, {}};
                    for (const auto& order : orderQueue) {
                        if (order->getVirtualQuantity() > 0) {
                            level.available_orders.emplace_back(order);
                        }
                    }
                    if (!level.available_orders.empty()) {
                        legLevels[i].emplace_back(level);
                    }
                }
            } else {
                // Negative Weight: Short. Look at Bids
                for (const auto& [price, orderQueue] : ob.bids) {
                    PriceLevel level{price, {}};
                    for (const auto& order : orderQueue) {
                        if (order->getVirtualQuantity()  > 0) {
                            level.available_orders.emplace_back(order);
                        }
                    }
                    if (!level.available_orders.empty()) {
                        legLevels[i].emplace_back(level);
                    }
                }
            }
        }

        StrategyInstance instance(strategy, 0);
        uint32_t totalArbitrageSize = 0;

        bool profitable = true;
        vector<size_t> currentLevelIndex(strategy.num_legs, 0);

        int64_t totalPnL = 0;

        while (profitable) {
            int64_t unit_pnl = 0;
            vector<uint32_t> levelQuantities;
            bool validLevels = true;

            for (size_t i = 0; i < legs.size(); i++) {
                if (currentLevelIndex[i] >= legLevels[i].size()) {
                    validLevels = false;
                    break;
                }
            }

            if (!validLevels) break;

            // is 1 unit of the arb profitable at this level?
            for (size_t i = 0; i < legs.size(); i++) {
                const auto& [legname, ob, position] = legs[i];
                const auto& level = legLevels[i][currentLevelIndex[i]];

                // Define portfolio: if we buy a position, we pay. If we sell, we get paid. We want instantaneous PnL
                // FIX PRICES!
                if (position > 0) {
                    // Long
                    unit_pnl -= level.price ;
                } else {
                    // Short
                    unit_pnl += level.price ;
                }

                // Calculate max virtual quantity by position size
                uint32_t availableQty = level.getAvailableQuantity();
                uint32_t curQty = availableQty / abs(position);
                levelQuantities.emplace_back(curQty);
            }

            if (unit_pnl > 0) {
            uint32_t maxQty = *min_element(levelQuantities.begin(), levelQuantities.end());

                if (maxQty > 0) {
                    // Record the trades and reduce virtual quantities
                    // Iterate through all the legs to find the price quantity and max size of each leg
                    for (size_t i = 0; i < legs.size(); i++) {
                        const auto& [hr_ticker, ob, position] = legs[i];
                        auto& level = legLevels[i][currentLevelIndex[i]];
                        uint64_t quantityNeeded = maxQty * abs(position);
                        uint64_t quantityRemaining = quantityNeeded;

                        // Record price for this leg
                        instance.level_sizes.emplace_back(make_tuple(hr_ticker, level.price, quantityNeeded));

                        // Reduce virtual quantities from orders at this level
                        for (auto& order : level.available_orders) {
                            if (quantityRemaining == 0) break;

                            if (order->getVirtualQuantity() == 0) {
                                continue;
                            }

                            uint64_t reduceAmount = std::min(static_cast<uint64_t>(order->getVirtualQuantity()), quantityRemaining);
                            order->reduceVirtualQuantity(reduceAmount);
                            quantityRemaining -= reduceAmount;

                            // Record order ID used
                            // instance.order_id_used.push_back(order->get_id());
                        }

                        // Remove orders with no remaining virtual quantity
                        level.available_orders.erase(
                            std::remove_if(level.available_orders.begin(), level.available_orders.end(),
                                [](const auto& order) { return order->getVirtualQuantity() <= 0; }),
                            level.available_orders.end()
                        );

                        // Move to next price level if current has zero quantity
                        if (level.available_orders.empty()) {
                            currentLevelIndex[i]++;
                        }
                    }

                    totalPnL += unit_pnl * static_cast<int64_t>(maxQty);

                    totalArbitrageSize += maxQty;
                } else {
                    profitable = false;
                }
            } else {
                profitable = false;
            }
        }

        instance.setFinalPortfolio(totalArbitrageSize);
        instance.set_pnl(totalPnL);

        return instance;

    };


private:

// List of all strategies in play
list<StrategyInstance> strategies;
// Maps each ticker to all the strategies it is a part of.
unordered_map<string, list<Portfolio>> ticker_to_strat_map;

};


#endif //STRATEGY_HPP
