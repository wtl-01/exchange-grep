// Compile:
// g++ -Wextra -O2 -o bbo_parser bbo_printer.cpp order_book_manager.cpp order_book.cpp -lpcap -std=c++20 -I include
// use -g for debug symbols in gdb
// Run:
// ./bbo_parser ../../md/md-cfe-tsp05-eth7-d-2024-41-ok.pcap

#include <cstdint>

#include "cfepitch.h"
#include "stopwatch.hpp"
#include "order_structures.hpp"
#include "cxxopts.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <order_book_manager.hpp>
#include <pcap.h>
#include <unordered_map>

using namespace std;

uint32_t gNextExpectedPacketSeqNum{0};

bool gap_detected{false};
bool trading_active{false};

// BBO TIME FIELDS
string currentDate;
string currentTradeDate; 
uint64_t time_offset;

OrderBookManager obm;
unordered_map<string, string> exchg_to_readable;
unordered_map<string, unordered_map<string, string>> symbol_to_readable_dict;


vector<string> BBO_Entries;

map<uint8_t, string> messageTypeInfo = {
        {0x20, "Time"},
        {0x21, "AddOrderLong"},
        {0x22, "AddOrderShort"},
        {0x23, "OrderExecuted"},
        {0x25, "ReduceSizeLong"},
        {0x26, "ReduceSizeShort"},
        {0x27, "ModifyOrderLong"},
        {0x28, "ModifyOrderShort"},
        {0x29, "DeleteOrder"},
        {0x2A, "TradeLong"},
        {0x2B, "TradeShort"},
        {0x2C, "TradeBreak"},
        {0x2D, "EndOfSession"},
        {0x31, "TradingStatus"},
        {0x97, "UnitClear"},
        {0xB1, "TimeReference"},
        {0xB9, "Settlement"},
        {0xBA, "EndOfDaySummary"},
        {0xBB, "FuturesInstrumentDefinition"},
        {0xBC, "TransactionBegin"},
        {0xBD, "TransactionEnd"},
        {0xBE, "PriceLimits"},
        {0xD3, "OpenInterest"}
};

// Function to convert currentTradeDate, currentTimeInSecs, and timeOffset to a timestamp string
string getTimestampString(const string &Date, uint32_t timeInSecs, uint32_t timeOffset)
{
        // Parse the trade date (assuming format "YYYYMMDD")
        tm tmDate = {};
        istringstream ss(Date);
        ss >> get_time(&tmDate, "%Y%m%d");

        // Add the time in seconds to the tm structure
        tmDate.tm_sec = timeInSecs % 60;
        tmDate.tm_min = (timeInSecs / 60) % 60;
        tmDate.tm_hour = (timeInSecs / 3600) % 24;

        // Format the timestamp string with nanoseconds
        ostringstream timestamp;
        timestamp << put_time(&tmDate, "%Y-%m-%d %H:%M:%S");
        timestamp << '.' << setfill('0') << setw(9) << timeOffset;
        return timestamp.str();
}

void cout_print_bbo(const uint64_t pktSeqNum, uint64_t msgSeqNum, string ticker, char& oper, BestBidOffer bbo, string srcTime, string trade_status)
{
        ostringstream oss;
        oss << srcTime << ","
                << pktSeqNum << ","
                << msgSeqNum << ","
                << oper << ","
                << symbol_to_readable_dict[ticker]["name"] << "," // Human Readable Ticker
                << static_cast<double>(bbo.best_bid_price)*10e-3  << ',' // BBO FIELDS
                << bbo.best_bid_amount << ',' // BBO FIELDS
                << static_cast<double>(bbo.best_ask_price)*10e-3  << ',' // BBO FIELDS
                << bbo.best_ask_amount << ',' // BBO FIELDS
                << trade_status << endl; // Trading status
        BBO_Entries.push_back(oss.str());
}

int process_message(const uint64_t pktSeqNum, uint64_t msgSeqNum, const u_char *message, int msg_type)
{
        switch (msg_type)
        {
                // NON-ORDER MESSAGES
                // The following message types do not alter book state.  They may be used
                // for information purposes in some Feedhandler implementations, or may be
                // ignored altogether in other, minimalistic, implementations.

        case 0x20: // Time
        {
                /*
                From the CFE PITCH Spec:

                "A Time message is immediately generated and sent when there is a PITCH
                event for a given clock second. If there is no PITCH event for a given clock
                second, then no Time message is sent for that second. All subsequent time
                offset fields for the same unit will use the new Time value as the base
                until another Time message is received for the same unit. The Time field is
                the number of seconds relative to midnight Central Time, which is provided
                in the Time Reference message. The Time message also includes the Epoch Time
                field, which is the current time represented as the number of whole seconds
                since the Epoch (Midnight January 1, 1970)."

                Additional notes:

                Most messages contain "TimeOffset" field with nanoseconds-only part of time.
                The "seconds" part is established by this Time message.  So Feedhandler
                should keep track of this "seconds" part and update it with every Time
                message.
                */

                Time m = *(Time *)message;
                time_offset = (uint64_t) m.Time;
                obm.set_current_state_timestamp(time_offset);

                break;
        }

        case 0x97: // UnitClear
        {
                /*
                From the CFE PITCH Spec:

                "The Unit Clear message instructs feed recipients to clear all orders for
                the CFE book in the unit specified in the Sequenced Unit Header. It would be
                distributed in rare recovery events such as a data center fail-over. It may
                also be sent on system startup (after daily restart) when there are no
                persisted GTCs or GTDs."

                Additional notes:

                Under normal conditions, this message is never seen.
                */

                UnitClear m = *(UnitClear *)message;
                break;
        }

        case 0xB1: // TimeReference
        {
                /*
                From the CFE PITCH Spec:

                "The Time Reference message is used to provide a midnight reference point
                for recipients of the feed. It is sent whenever the system starts up and
                when the system crosses a midnight boundary. All subsequent Time messages
                for the same unit will the use the last Midnight Reference until another
                Time Reference message is received for that unit. The Time Reference message
                includes the Trade Date, so most other sequenced messages will not include
                that information.""

                Additional notes:

                Most likely this is never needed.
                */

                TimeReference m = *(TimeReference *)message;
                currentTradeDate = to_string(m.TradeDate);

                time_t midnightRefTime = m.MidnightReference;
                std::tm *tmPtr = std::gmtime(&midnightRefTime);
                time_t GMT_time = mktime(tmPtr);

                if (tmPtr)
                {
                        char buffer[80];
                        std::strftime(buffer, sizeof(buffer), "%Y%m%d", tmPtr);
                        currentDate = buffer;
                }
                else
                {
                        std::cerr << "Failed to convert MidnightReference timestamp." << std::endl;
                }
                if (!trading_active)
                {
                        obm.set_start_time(GMT_time);
                        obm.set_current_state_timestamp(0);
                }

                break;
        }

        case 0xBB: // FuturesInstrumentDefinition
        {
                /*
                From the CFE PITCH Spec:

                "The Futures Instrument Definition message can be sent as a sequenced
                message or an unsequenced message. It is sent as a sequenced message when
                the system starts up at the beginning of a trading session or if an
                instrument is created or modified during a trading day. A new sequenced
                message may be sent for a Symbol that does not visibly change any attribute.
                One un-sequenced Futures Instrument Definition message for each Symbol is
                also sent in a continuous loop, which completes approximately once every
                minute."

                Additional notes:

                In theory, new instruments may be added intra-day and their instrument
                definitions will be distributed via this message - but in practice this does
                not happen.  So there is no need to continuously listen to instrument
                definitions.  Mostly likely, these would be used just once to get instrument
                descriptions - in approximately one minute all definitions can be received.
                Or even more likely, the client system would already have all definitions
                built by a different component and stored in e.g. configuration file; in
                that case these messages can be completely ignored.
                */

                FuturesInstrumentDefinition m = *(FuturesInstrumentDefinition *)message;

                // TODO: handle instrument definitions

                string symbol(reinterpret_cast<char*>(m.Symbol), 6);
                string report_symbol = "";
                for(uint8_t& i : m.ReportSymbol){
                    char c = static_cast<char>(i);
                    if(isalnum(c)){
                        report_symbol += c;
                    }else{
                        break;
                    }
                }

                if(m.LegCount <= 0){
                    int year_code = (m.ExpirationDate / 10000) % 10;
                    int month = (m.ExpirationDate / 100) % 100;
                    const char* monthCodes = "FGHJKMNQUVXZ";
                    char month_code;
                    if (month >= 1 && month <= 12) {
                            month_code = monthCodes[month - 1];
                    }else{
                            cerr << "month_code not recognizable error";
                            exit(-1);
                    }
                    string readable_code = report_symbol + month_code + to_string(year_code);
                    //{"name":"VXV4", "id":"000XIZ", "legs":{}, "tick":0.05, "size":1000, "product":"VX/V4", "root":"VX", "expiry":"2024-10-16", "description":"VX Monthly Oct 2024", "test":false},
                    char is_test = static_cast<char>(m.ListingState);
                    string product = report_symbol + "/" + month_code + to_string(year_code);

                    symbol_to_readable_dict[symbol]["name"] = readable_code;
                    symbol_to_readable_dict[symbol]["legs"] = "";
                    symbol_to_readable_dict[symbol]["product"] = product;

                    /*
                    cout << symbol_to_readable_dict[symbol]["name"] << endl;
                    cout << symbol_to_readable_dict[symbol]["legs"] << endl;
                    cout << symbol_to_readable_dict[symbol]["tick"] << endl;
                    cout << symbol_to_readable_dict[symbol]["size"] << endl;
                    cout << symbol_to_readable_dict[symbol]["product"] << endl;
                    cout << symbol_to_readable_dict[symbol]["root"] << endl;
                    cout << symbol_to_readable_dict[symbol]["expiry"] << endl;
                    cout << symbol_to_readable_dict[symbol]["test"] << endl;
                    cout << "--------" << endl;
                    */


                }else{ //spread instrument
                    //{"name":"VXV4", "id":"000XIZ", "legs":{}, "tick":0.05, "size":1000, "product":"VX/V4", "root":"VX", "expiry":"2024-10-16", "description":"VX Monthly Oct 2024", "test":false}
                    string product = "";
                    string legs = "";
                    string readable_code = "";
                    uint8_t* baseAddress = (uint8_t*) message + m.LegOffset - 2;
                    for(int i = 0; i < m.LegCount; ++i){
                        int32_t leg_ratio = *(uint32_t*)(baseAddress + 10 * i);
                        uint8_t leg_symbol_arr[6];
                        memcpy(leg_symbol_arr, baseAddress+4+10*i, 6);
                        string leg_symbol(reinterpret_cast<char*>(leg_symbol_arr), 6);
                        product += symbol_to_readable_dict[leg_symbol]["name"];
                        if(leg_ratio == 1){
                            readable_code += "+";
                            legs += "+" + leg_symbol;
                            product += ":1:B";
                        }else if(leg_ratio == -1){
                            readable_code += "-";
                            legs += "-" + leg_symbol;
                            product += ":1:S - ";
                        }else{
                            cerr << "an error occur (leg ratio error)";
                        }
                        readable_code += symbol_to_readable_dict[leg_symbol]["name"];
                    }
                    symbol_to_readable_dict[symbol]["name"] = readable_code;
                    symbol_to_readable_dict[symbol]["legs"] = legs;
                    symbol_to_readable_dict[symbol]["product"] = product;
                    //cout << "readable_code " << symbol_to_readable_dict[symbol]["name"] << endl;
                    //cout << product << endl;
                }

                char is_test = static_cast<char>(m.ListingState);
                double tick = static_cast<double>(m.PriceIncrement) / 10000;
                int contract_size = m.ContractSize;
                string date_str = to_string(m.ExpirationDate);
                string expiry = date_str.substr(0,4) + "-" + date_str.substr(4,2) + "-" + date_str.substr(6, 2);

                string root;
                for(int i = 0; i < report_symbol.length(); i++){
                    if(!isalpha(report_symbol[i])){
                        root = report_symbol.substr(0, i);
                        break;
                    } else if(i == report_symbol.length() - 1){
                        root = report_symbol;
                    }
                }
                symbol_to_readable_dict[symbol]["tick"] = to_string(tick);
                symbol_to_readable_dict[symbol]["size"] = to_string(contract_size);
                symbol_to_readable_dict[symbol]["expiry"] = expiry;
                symbol_to_readable_dict[symbol]["root"] = root;
                if(is_test == 'T'){
                    symbol_to_readable_dict[symbol]["test"] = "true";
                } else{
                    symbol_to_readable_dict[symbol]["test"] = "false";
                }

                break;
        }

        case 0xBE: // PriceLimits
        {
                /*
                From the CFE PITCH Spec:

                "The Price Limits message is sent out at the start of a session for products
                subject to price limits per the contract specifications. The Price Limits
                message does not signal whether price limits are in effect for that symbol;
                it simply provides those values for when they are in effect. If multiple
                Price Limits messages are received for the same Symbol, the most recent
                values will override the previous values."

                Additional notes:

                In practice, price limits are so wide that no CFE instrument has reached its
                price limits ever in the entire history of trading.  So these are of
                questionable value.  They can also be trivially calculated based on the
                prior closing price.  Nevertheless, these may be used to e.g. visualize the
                limits on the client GUI.
                */

                PriceLimits m = *(PriceLimits *)message;
                break;
        }

        case 0xBC: // TransactionBegin
        {
                /*
                From the CFE PITCH Spec:

                "The Transaction Begin message indicates any subsequent messages, up to the
                accompanying Transaction End message, are all part of the same transaction
                block. One example of where this might be used is when a single aggressive
                order executes against several resting orders. All PITCH messages
                corresponding to such an event would be included between a Transaction Begin
                and Transaction End. It is important to note that any PITCH Message Type may
                be included in a transaction block and there is no guarantee that the
                messages apply to the same price level or even the same Symbol. Transaction
                Begin messages do not alter the book and can be ignored if messages are
                being used solely to build a book. Feed processors can use a transaction
                block as a trigger to postpone publishing a quote update until the end of
                the transaction block. In the prior example of a single aggressive order
                executing against multiple resting orders, a top of book feed would be able
                to publish a single trade message and quote update resulting from multiple
                Order Executed messages once it finished processing all of the messages
                within the transaction block."

                Additional notes:

                In practice, messages included in the TransactionBegin-TransactionEnd block
                are almost always OrderExecuted which result from one large agressor
                executing against multiple smaller resting orders - the same example as used
                in the CFE PITCH spec.  This is really a *single* match event, but due to
                system design CFE published it as multiple events - one per resting order -
                all with the same Exchange timestamp.  In practice number of messages is
                often 10 to 20 but may also reach 100 or more.  Feedhandler should
                definitely avoid publishing updates to clients before it completes
                processing of a transaction block.
                */

                TransactionBegin m = *(TransactionBegin *)message;
                break;
        }

        case 0xBD: // TransactionEnd
        {
                /*
                See comments for TransactionBegin.
                */

                TransactionEnd m = *(TransactionEnd *)message;
                break;
        }

        case 0x31: // TradingStatus
        {
                /*
                From the CFE PITCH Spec:

                "The Trading Status message is used to indicate the current trading status
                of a Futures contract. A Trading Status message will be sent whenever a
                security’s trading status changes. If a Trading Status has not been received
                for a symbol, then the Trading Status for the symbol should be assumed to be
                “S = Suspended”."

                Additional notes:

                These are sent for every instrument - both simple and complex (spreads).  In
                general:
                  - Around 16:45 Central Time, instruments transition to "Q - Queing";
                orders can be added but no trading yet
                  - Around 17:00 Central Time, instruments transition to "T - Trading";
                trading is enabled
                  - Around 16:00 Central Time (next day), instruments transition to "S -
                Suspended"; trading is closed

                Note that some low-liquidity instruments, like exotic spreads or far-out
                outrights, may transition between Q and T also at seemingly random time
                during the trading day.
                */

                TradingStatus m = *(TradingStatus *)message;
                        string ticker = string(m.Symbol, 6);
                char trading_status = (char)m.TradingStatus;
                ExchangeStatus es = (trading_status == 'Q') ? ExchangeStatus::Q
                        : (trading_status == 'T') ? ExchangeStatus::T
                        : (trading_status == 'S') ? ExchangeStatus::S
                        : ExchangeStatus::S;

                obm.set_ticker_trading_status(ticker, es);
                break;
        }

        case 0x2A: // TradeLong
        case 0x2B: // TradeShort
        {
                /*
                From the CFE PITCH Spec:

                "The Trade message provides information about executions that occur off of
                the CFE book (such as ECRP/Block trades). Trade messages are necessary to
                calculate CFE execution data. Trade messages do not alter the book and can
                be ignored if messages are being used solely to build a book. The Order Id
                sent in a Trade message is obfuscated and will not tie back to any real
                Order Id sent back via a FIX or BOE order entry session."

                Additional notes:

                Most commonly, TradeLong/TradeShort messages are sent when a spread
                instrument trades.  CFE sends OrderExecuted for the spread itself, and
                TradeLong/TradeShort for each of the legs.  As noted in the Spec, these can
                be ignored for the purposes of book-building because the actual book which
                trades is the spread book, not the legs. They may be useful for some other
                purposes like calculating leg trading volume.  When these are sent, almost
                always they are TradeShort and not TradeLong.
                */

                if (msg_type == 0x2A)
                {
                        TradeLong m = *(TradeLong *)message;
                }
                if (msg_type == 0x2B)
                {
                        TradeShort m = *(TradeShort *)message;
                }
                break;
        }

        case 0x2C: // TradeBreak
        {
                /*
                From the CFE PITCH Spec:

                "The Trade Break message is sent whenever an execution on CFE is broken.
                Trade breaks are rare and only affect applications that rely upon CFE
                execution-based data. A Trade Break followed immediately be a new Trade with
                the same Execution Id indicates that a trade correction has occurred.
                Applications that simply build a CFE book can ignore Trade Break messages."

                Additional notes:

                As noted, these are rare.  There are none in the reference market data file
                for trade date 2023-11-16.
                */

                TradeBreak m = *(TradeBreak *)message;
                break;
        }

        case 0xB9: // Settlement
        {
                /*
                From the CFE PITCH Spec:

                "Settlement messages are used to provide information concerning indicative,
                approved, or corrected daily and final settlement prices for CFE products.
                An indicative daily settlement price (Issue = I) is calculated by the system
                and sent immediately after an instrument closes trading but before the
                settlement price is approved. An approved settlement price (Issue = S) is
                sent once the CFE Trade Desk approves a settlement price for an instrument.
                If there is an error in the approved settlement price, then it may be
                re-issued (Issue = R). For symbols that settle each day using VWAP, the
                system will begin disseminating an intermediate indicative price update
                (Issue = i) at 2:59:35 p.m. CT (following the first interval of the VWAP
                calculation) that will be sent every five seconds, leading up to the receipt
                of the indicative daily settlement price (Issue = I)."

                Additional notes:

                Typically these are informational-only but there may be a trading strategy
                client interested in the indicative settlement prices.  Thus, a Feedhandler
                may want to forward these to clients.
                */

                Settlement m = *(Settlement *)message;
                break;
        }

        case 0xD3: // OpenInterest
        {
                /*
                From the CFE PITCH Spec:

                "The Open Interest message is sent to communicate a symbol’s open interest,
                usually for the prior trading date. This message will be sent when open
                interest information is made available to CFE and may be sent multiple times
                if there are changes to the open interest for a symbol. The open interest is
                also populated in the End of Day Summary message."

                Additional notes:

                Most likely these are informational-only.  Maybe a client may want to store
                these daily values into the database for some later analysis.  These may
                usually be obtained from other sources, also.
                */

                OpenInterest m = *(OpenInterest *)message;
                break;
        }

        case 0xBA: // EndOfDaySummary
        {
                /*
                From the CFE PITCH Spec:

                "The End of Day Summary is sent immediately after trading ends for a symbol.
                No more Market Update messages will follow an End of Day Summary for a
                particular symbol. A value of zero in the Total Volume field means that no
                volume traded on that symbol for the day. The Total Volume field reflects
                all contracts traded during the day. Block, ECRP, and Derived (effective
                12/11/23) trades are included in the Total Volume field, but they are also
                reported separately to provide more detail.""

                Additional notes:

                Similar to OpenInterest, these would most likely be informational-only.  May
                be useful to store daily values to some database.
                */

                EndOfDaySummary m = *(EndOfDaySummary *)message;
                break;
        }

        case 0x2D: // EndOfSession
        {
                /*
                From the CFE PITCH Spec:

                "The End of Session message is sent for each unit when the unit shuts down.
                No more sequenced messages will be delivered for this unit, but heartbeats
                from the unit may be received."

                Additional notes:

                Very last sequenced message on the feed.  Probably not very useful and can
                be ignored, since it's pretty clear when things shut down based on other
                data.  Perhaps could be used to double-check data itegrity by verifying that
                it is indeed the last message.
                */

                EndOfSession m = *(EndOfSession *)message;
                break;
        }

                // ORDER MESSAGES
                // The following messages do alter book state.  In order correctly build
                // order books and maintain data integrity, it is essential to process every
                // message correctly.  Even a single missed message will likely result in
                // loss of data integrity.  Appropriate checks may and should be used in
                // code.  For instance, all of OrderExecuted, ReduceSize, ModifyOrder, and
                // DeleteOrder messages refer to the OrderId of an order which must exist;
                // if Feedhandler is not able to find such OrderId it most likely means that
                // it has missed on incorrectly processed some earlier message(s).
                // OrderExecuted should only occur to orders at the top of the FIFO queue,
                // and so on.  It is recommended that Feedhandler implements data integrity
                // checks whether in a form of asserts, exceptions, or log messages -
                // especially during the development stages while the code is not yet
                // proven.

        case 0x21: // AddOrderLong
        case 0x22: // AddOrderShort
        {
                /*
                From the CFE PITCH Spec:

                "An Add Order message represents a newly accepted visible order on the CFE
                book. It includes a day-specific Order Id assigned by CFE to the order."

                Additional notes:

                AddOrderLong/AddOrderShort is what adds orders to the books.  In the
                reference market data file, there are about 700 thousand AddOrder messages.
                Almost all of them are AddOrderShort - but once in a while CFE sends
                AddOrderLong to keep things interesting, so it cannot be ignored.  It may be
                useful to process these two together as a single message type to avoid code
                duplication.

                The first AddOrder messages for the day are typically sent around 16:07.
                These are GTC orders that are left in books being restored.  There's usually
                about 15 thousand GTC orders.  In order to maintain data integrity, it is
                essential to listen to these GTC-restoring day orders or have them in PCAP.
                Thus, a full day PCAP should start prior to this approximately 16:07 time.
                All GTC-restoring AddOrder messages have the same Exchange timestamp and are
                sent closely together, possibly at the maximum line rate of 10 Gbps.  Thus,
                15 thousand or so messages may be received during a single millisecond
                creating one of the peak daily load spikes.  It is, of course, important
                that a Feedhandler is able to process this spike and does not drop any of
                these messages.

                It may be important for client strategies to know whether orders are GTC or
                not.  So a Feedhandler may want to mark orders added during this
                GTC-restoration cycle as GTC, as opposed to DAY orders added later.

                In total, there are about 700 thousand AddOrder messages for the day.
                */

                if (msg_type == 0x21)
                {
                        AddOrderLong m = *(AddOrderLong *)message;
                        string ticker = string(m.Symbol, 6);
                        if (!obm.contains_orderbook(ticker))
                        {
                                obm.add_orderbook(ticker);
                        }

                        Side side = m.SideIndicator == 'B' ? Side::Buy : Side::Sell;
                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        pair<bool, BestBidOffer> bbo_entry = obm.handle_CBOE_add_order(m.OrderId, ticker, m.Price, m.Quantity, side);
                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";

                                char oper = 'A';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                }
                if (msg_type == 0x22)
                {
                        AddOrderShort m = *(AddOrderShort *)message;

                        string ticker = string(m.Symbol, 6);
                        if (!obm.contains_orderbook(ticker))
                        {
                                obm.add_orderbook(ticker);
                        }

                        Side side = m.SideIndicator == 'B' ? Side::Buy : Side::Sell;
                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        pair<bool, BestBidOffer> bbo_entry =
                                obm.handle_CBOE_add_order(m.OrderId, ticker,
                                        static_cast<int64_t>(m.Price),
                                        static_cast<int32_t>(m.Quantity), side);
                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";

                                char oper = 'A';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                }
                break;
        }

        case 0x23: // OrderExecuted
        {
                /*
                From the CFE PITCH Spec:

                "Order Executed messages are sent when an order on the CFE book is executed
                in whole or in part. The execution price equals the limit order price found
                in the original Add Order message or the limit order price in the latest
                Modify Order message referencing the Order Id."

                Additional notes:

                CFE is strictly a First-In-First-Out (FIFO) market, so resting order
                priority is based first on price level and then on order entry time.  When
                an resting order is executed, it must be the first in the FIFO queue.  This
                fact may be used as [additional] data integrity check within the Feedhandler
                - if OrderExecuted is received for an order which is not at the top price
                level, or is on the top price level but is not front of the FIFO queue - it
                indicates a data itegrity issue.

                In total, there are about 40 thousand OrderExecuted messages for the day.
                */

                        OrderExecuted m = *(OrderExecuted *)message;

                        Order curr_order = obm.get_order(m.OrderId);
                        string ticker = curr_order.getSymbol();

                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        pair<bool, BestBidOffer> bbo_entry =
                                obm.handle_CBOE_order_executed(m.OrderId, m);
                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";

                                char oper = 'E';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                break;
        }

        case 0x25: // ReduceSizeLong
        case 0x26: // ReduceSizeShort
        {
                /*
                From the CFE PITCH Spec:

                "Reduce Size messages are sent when a visible order on the CFE book is
                partially reduced."

                Additional notes:

                Note that the spec says "partially reduced" - so these are sent only when
                there is an order remainder of at least 1, for example, order quantity is
                reduced from 7 to 1, and price remains the same.  On CFE, it is possible to
                send a "modify" request reducing the quantity to zero, but in that case CFE
                sends a "DeleteOrder" message rather than reduce.

                Similar to other Long/Short message flavors, ReduceSize is almost always
                ReduceSizeShort; in the reference file there are no ReduceSizeLong messages.
                However, there are no guarantees that CFE does not decide to send a
                ReduceSizeLong just for the fun of it, so both must be processed.

                There are about 30 thousand ReduceSize messages in the reference MD file.
                */

                if (msg_type == 0x25)
                {
                        ReduceSizeLong m = *(ReduceSizeLong *)message;

                        Order curr_order = obm.get_order(m.OrderId);
                        string ticker = curr_order.getSymbol();

                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        pair<bool, BestBidOffer> bbo_entry =
                                obm.handle_CBOE_reduce_size(m.OrderId, m.CancelledQuantity);
                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";

                                char oper = 'R';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                }
                if (msg_type == 0x26)
                {
                        ReduceSizeShort m = *(ReduceSizeShort *)message;

                        Order curr_order = obm.get_order(m.OrderId);
                        string ticker = curr_order.getSymbol();

                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        pair<bool, BestBidOffer> bbo_entry =
                                obm.handle_CBOE_reduce_size(m.OrderId, static_cast<uint32_t>(m.CancelledQuantity));
                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";

                                char oper = 'R';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                }
                break;
        }

        case 0x27: // ModifyOrderLong
        case 0x28: // ModifyOrderShort
        {
                /*
                From the CFE PITCH Spec:

                "The Modify Order message is sent whenever an open order is visibly
                modified. The Order Id refers to the Order Id of the original Add Order
                message.  Note that Modify Order messages that appear to be “no ops” (i.e.
                they do not appear to modify any relevant fields) will still lose priority."

                Additional notes:

                Similar to other Long/Short messages flavors, almost all modifies are
                ModifyOrderShort - there are about 360 thousand of them in the reference MD
                file.  But ModifyOrderLong should also be handled, and there is in fact 1 of
                those in the reference file.

                ModifyOrder represent the most complex message for book-building purposes.
                Note that ModifyOrder may modify an order's (1) price only, (2) quantity
                only, (3) both price and quantity, or (4) nothing.  Case (4) is a "no-op
                modify" mentioned in the CFE spec and results in order being sent to the end
                of the FIFO queue.
                */

                if (msg_type == 0x27)
                {
                        ModifyOrderLong m = *(ModifyOrderLong *)message;

                        Order curr_order = obm.get_order(m.OrderId);
                        string ticker = curr_order.getSymbol();

                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        //ModifyFields mf = {m.Price, m.Quantity};
                        pair<bool, BestBidOffer> bbo_entry =
                                obm.handle_CBOE_modify_order(m.OrderId, m.Price, m.Quantity);

                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";

                                char oper = 'M';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                }
                if (msg_type == 0x28)
                {

                        ModifyOrderShort m = *(ModifyOrderShort *)message;


                        Order curr_order = obm.get_order(m.OrderId);
                        string ticker = curr_order.getSymbol();

                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        //ModifyFields mf = {m.Price, m.Quantity};
                        pair<bool, BestBidOffer> bbo_entry =
                                obm.handle_CBOE_modify_order(m.OrderId, static_cast<int64_t>(m.Price), static_cast<uint32_t>(m.Quantity));
                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";
                                char oper = 'M';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                }
                break;
        }

        case 0x29: // DeleteOrder
        {
                /*
                From the CFE PITCH Spec:

                "The Delete Order message is sent whenever a booked order is cancelled or
                leaves the order book. The Order Id refers to the Order Id of the original
                Add Order message. An order that is deleted from the book may return to the
                book later under certain circumstances. Therefore, a Delete Order message
                does not indicate that a given Order Id will not be sent again on a
                subsequent Add Order message."

                Additional notes:

                Note the second part of the above comment from PITCH spec - deleted order
                may return to the book under certain conditions.  This means that the same
                Order ID may be used *again* following the delete.  So, while Ordder IDs are
                unique among outstanding orders, they are not globally unique.

                DeleteOrder is the second-most common message after AddOrder, with about 670
                thousand DeleteOrder messages in the reference file.
                */

                        DeleteOrder m = *(DeleteOrder *)message;


                        Order curr_order = obm.get_order(m.OrderId);
                        string ticker = curr_order.getSymbol();


                        // This spits out whether this entry is a BBO we need to write to file.
                        obm.set_current_state_timestamp(static_cast<uint64_t>(m.TimeOffset));
                        //ModifyFields mf = {m.Price, m.Quantity};
                        pair<bool, BestBidOffer> bbo_entry =
                                obm.handle_CBOE_cancel_order(m.OrderId);
                        if (bbo_entry.first)
                        {
                                BestBidOffer bbo = bbo_entry.second;
                                ExchangeStatus es = obm.get_ticker_trading_status(ticker);
                                string srcTime = getTimestampString(currentDate, time_offset, m.TimeOffset);
                                string status = es == ExchangeStatus::S ? "S" : es == ExchangeStatus::T ? "T" : es == ExchangeStatus::Q ? "Q" : "I";
                                char oper = 'D';
                                cout_print_bbo(pktSeqNum, msgSeqNum, ticker, oper, bbo, srcTime, status);
                        }
                        break;
        }

        default:
        {
                cerr << "Message Processor: unknown message type " << msg_type << endl;
                exit(1);
        }
        }

        return 0;
}

int process_packet(const u_char *packet, struct pcap_pkthdr header)
{
        /*
        All PCAP data has the following header lengths:
        Ethernet: 14 bytes
        IP      : 20 bytes
        UDP     : 8 bytes

        PITCH payload starts at byte 43.
        */

        int offset = 42;

        SequencedUnitHeader suHeader = *(SequencedUnitHeader *)(packet + offset);

        // Skip PITCH packets for Unit other than 1, unsequenced PITCH packets, and
        // those with zero messages (heartbeats)
        if (suHeader.HdrUnit != 1 || suHeader.HdrSequence == 0 || suHeader.HdrCount == 0)
        {
                return 1;
        }

        // Gap detection
        if (gNextExpectedPacketSeqNum)
        {
                //Time sequences will be reset to 1 each day when feed startup
                if (suHeader.HdrSequence == 1)
                {
                        cerr << "Packet Processor: feed startup detected" << endl;
                }
                else if (suHeader.HdrSequence != gNextExpectedPacketSeqNum)
                {
                        cerr << "Packet Processor: gap detected. Expected seq " << gNextExpectedPacketSeqNum
                            << " actual: " << suHeader.HdrSequence << endl;
                        gap_detected = true;
                        exit(1);
                }
        }

        gNextExpectedPacketSeqNum = suHeader.HdrSequence + suHeader.HdrCount;
        uint64_t pktSeqNum = suHeader.HdrSequence;
        uint64_t msgSeqNum = suHeader.HdrSequence;
        // First message in packet
        offset += sizeof(SequencedUnitHeader);
        MessageHeader msgHeader = *(MessageHeader *)(packet + offset);

        //process_message(packet + offset + 2, msgHeader.MsgType);
        process_message(pktSeqNum, msgSeqNum, packet + offset + 2, msgHeader.MsgType);



        // All remaining messages in packet
        for (int j = 0; j < suHeader.HdrCount - 1; j++)
        {
                msgSeqNum ++;
                offset += msgHeader.MsgLen;
                msgHeader = *(MessageHeader *)(packet + offset);
                process_message(pktSeqNum, msgSeqNum, packet + offset + 2, msgHeader.MsgType);
        }

        return 0;
}

int main(int argc, char *argv[])
{
        if (argc < 2)
        {
                std::cerr << "Usage: " << argv[0] << " md.pcap" << std::endl;
                return 1;
        }

        StopWatch sw2 = StopWatch();

        std::string pcapFileName = argv[1];

        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t *pcap;

        sw2.Start();

        pcap = pcap_open_offline(pcapFileName.c_str(), errbuf);
        if (pcap == nullptr)
        {
                std::cerr << "Error opening pcap file: " << errbuf << std::endl;
                return 1;
        }

        struct pcap_pkthdr header;
        const u_char *packet;

        StopWatch sw = StopWatch();
        sw.Start();
        while ((packet = pcap_next(pcap, &header)) != nullptr)
        {
                process_packet(packet, header);
        }

        pcap_close(pcap);
        sw.Stop();

        string filename = "BBO_2024-41.csv";
        std::ofstream csvFile(filename);
        if (!csvFile.is_open()) {
                std::cerr << "Error opening file: " << filename << std::endl;
                return -1;
        }

        csvFile << "Time,PktSeqNum,MsgSeqNum,MsgType,Symbol,BidPrice,BidQuantity,AskPrice,AskQuantity,TradingStatus" << endl;

        for (const auto& rec : BBO_Entries) {
                csvFile << rec;
        }

        csvFile.close();
        if (csvFile.fail()) {
                std::cerr << "Error writing to file: " << filename << std::endl;
        }
        else {
                std::cerr << "BBO data written to " << filename << std::endl;
        }


        sw2.Stop();

        const double loop_elapsed_time = sw.GetTime();
        const double total_elapsed_time = sw2.GetTime();
        std::cerr << "--------------------------------------\n";
        std::cerr << "Loop Duration: " << loop_elapsed_time << "s" << std::endl;
        std::cerr << "Duration incl I/O: " << total_elapsed_time << "s" << std::endl;
        std::cerr << "--------------------------------------\n";

        return 0;
}