#pragma once
#include <cstdint>
#include <sstream>
using namespace std;

/*
Reference:

CFE PITCH Specs v1.2.5 2023-11-28.pdf
*/

struct [[gnu::packed]] SequencedUnitHeader
{
    uint16_t HdrLen;
    uint8_t HdrCount;
    uint8_t HdrUnit;
    uint32_t HdrSequence;
};
static_assert(sizeof(SequencedUnitHeader) == 8, "SequencedUnitHeader struct must be 8 bytes");

struct [[gnu::packed]] MessageHeader
{
    uint8_t MsgLen;
    uint8_t MsgType;
};
static_assert(sizeof(MessageHeader) == 2, "MessageHeader struct must be 2 bytes");

struct [[gnu::packed]] Time // 0x20
{
    uint32_t Time;
    uint32_t EpochTime;
};
static_assert(sizeof(Time) == 8, "Time struct must be 8 bytes");

struct [[gnu::packed]] UnitClear // 0x97
{
    uint32_t TimeOffset;
};
static_assert(sizeof(UnitClear) == 4, "UnitClear struct must be 4 bytes");

struct [[gnu::packed]] TimeReference // 0xB1
{
    uint32_t MidnightReference;
    uint32_t Time;
    uint32_t TimeOffset;
    uint32_t TradeDate;
};
static_assert(sizeof(TimeReference) == 16, "TimeReference struct must be 16 bytes");

struct [[gnu::packed]] FuturesInstrumentDefinition // 0xBB
{
    uint32_t TimeOffset;
    uint8_t Symbol[6];
    int32_t UnitTimestamp;
    uint8_t ReportSymbol[6];
    uint8_t FuturesFlags;
    uint32_t ExpirationDate;
    uint16_t ContractSize;
    uint8_t ListingState;
    uint64_t PriceIncrement;
    uint8_t LegCount;
    uint8_t LegOffset;
    uint8_t VarianceBlockOffset;
    uint32_t ContractDate;
    // Optional fields follow
};
static_assert(sizeof(FuturesInstrumentDefinition) == 43, "FuturesInstrumentDefinition struct must be 43 bytes");

struct [[gnu::packed]] PriceLimits // 0xBE
{
    uint32_t TimeOffset;
    uint8_t Symbol[6];
    int64_t UpperPriceLimit;
    int64_t LowerPriceLimit;
};
static_assert(sizeof(PriceLimits) == 26, "PriceLimits struct must be 23 bytes");

struct [[gnu::packed]] AddOrderLong // 0x21
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint8_t SideIndicator;
    uint32_t Quantity;         // uint16_t in AddOrderShort
    char Symbol[6];
    //uint16_t SymbolHi;
    //uint32_t SymbolLow;
    int64_t Price;             // uint16_t in AddOrderShort

    string txt(){
        stringstream ss;
        ss << "AddOrderLong {OrderId: " << OrderId << "; SideIndicator: " << SideIndicator <<  "; Quantity: "
            << Quantity << "; Symbol: [" << Symbol << "]; Price: " << Price << "}";
        return ss.str();
    }
};
static_assert(sizeof(AddOrderLong) == 31, "AddOrderLong struct must be 31 bytes");

struct [[gnu::packed]] AddOrderShort // 0x22
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint8_t SideIndicator;
    uint16_t Quantity;
    char Symbol[6];
    //uint16_t SymbolHi;
    //uint32_t SymbolLow;
    int16_t Price;

    string txt(){
        stringstream ss;
        ss << "AddOrderShort {OrderID: " << OrderId << "; SideIndicator: " << SideIndicator <<  "; Quantity: "
            << Quantity << "; Symbol: [" << Symbol << "]; Price: " << Price << "}";
        return ss.str();
    }
};
static_assert(sizeof(AddOrderShort) == 23, "AddOrderShort struct must be 23 bytes");

struct [[gnu::packed]] OrderExecuted // 0x23
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint32_t ExecutedQuantity;
    uint64_t ExecutionId;
    uint8_t TradeCondition;

    string txt(){
        stringstream ss;
        ss << "OrderExecuted {OrderId: " << OrderId << "; ExecutedQuantity: " << ExecutedQuantity <<  "; ExecutionId: "
            << ExecutionId << "; TradeCondition: '" << TradeCondition << "'}";
        return ss.str();
    }
};
static_assert(sizeof(OrderExecuted) == 25, "OrderExecuted struct must be 25 bytes");

struct [[gnu::packed]] ReduceSizeLong // 0x25
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint32_t CancelledQuantity;

    string txt(){
        stringstream ss;
        ss << "ReduceSizeLong {OrderId: " << OrderId << "; CancelledQuantity: " << CancelledQuantity << "}";
        return ss.str();
    }
};
static_assert(sizeof(ReduceSizeLong) == 16, "ReduceSizeLong struct must be 16 bytes");

struct [[gnu::packed]] ReduceSizeShort // 0x26
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint16_t CancelledQuantity;

    string txt(){
        stringstream ss;
        ss << "ReduceSizeShort {OrderId: " << OrderId << "; CancelledQuantity: " << CancelledQuantity << "}";
        return ss.str();
    }
};
static_assert(sizeof(ReduceSizeShort) == 14, "ReduceSizeShort struct must be 14 bytes");

struct [[gnu::packed]] ModifyOrderLong // 0x27
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint32_t Quantity;
    int64_t Price;

    string txt(){
        stringstream ss;
        ss << "ModifyOrderLong {OrderId: " << OrderId << "; Quantity: " << Quantity <<  "; Price: " << Price << "}";
        return ss.str();
    }
};
static_assert(sizeof(ModifyOrderLong) == 24, "ModifyOrderLong struct must be 16 bytes");

struct [[gnu::packed]] ModifyOrderShort // 0x28
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint16_t Quantity;
    int16_t Price;

    string txt(){
        stringstream ss;
        ss << "ModifyOrderShort {OrderId: " << OrderId << "; Quantity: " << Quantity <<  "; Price: " << Price << "}";
        return ss.str();
    }
};
static_assert(sizeof(ModifyOrderShort) == 16, "ModifyOrderShort struct must be 16 bytes");

struct [[gnu::packed]] DeleteOrder // 0x29
{
    uint32_t TimeOffset;
    uint64_t OrderId;

    string txt(){
        stringstream ss;
        ss << "DeleteOrder {OrderId: " << OrderId << "}";
        return ss.str();
    }
};
static_assert(sizeof(DeleteOrder) == 12, "DeleteOrder struct must be 12 bytes");

struct [[gnu::packed]] TradeLong // 0x2A
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint8_t SideIndicator;
    uint32_t Quantity;
    uint8_t Symbol[6];
    int64_t Price;
    uint64_t ExecutionId;
    uint8_t TradeCondition;
};
static_assert(sizeof(TradeLong) == 40, "TradeLong struct must be 40 bytes");

struct [[gnu::packed]] TradeShort // 0x2B
{
    uint32_t TimeOffset;
    uint64_t OrderId;
    uint8_t SideIndicator;
    uint16_t Quantity;
    uint8_t Symbol[6];
    int16_t Price;
    uint64_t ExecutionId;
    uint8_t TradeCondition;
};
static_assert(sizeof(TradeShort) == 32, "TradeShort struct must be 32 bytes");

struct [[gnu::packed]] TransactionBegin // 0xBC
{
    uint32_t TimeOffset;
};
static_assert(sizeof(TransactionBegin) == 4, "TransactionBegin struct must be 4 bytes");

struct [[gnu::packed]] TransactionEnd // 0xBD
{
    uint32_t TimeOffset;
};
static_assert(sizeof(TransactionEnd) == 4, "TransactionEnd struct must be 4 bytes");

struct [[gnu::packed]] TradeBreak // 0x2C
{
    uint32_t TimeOffset;
    uint64_t ExecutionId;
};
static_assert(sizeof(TradeBreak) == 12, "TradeBreak struct must be 12 bytes");

struct [[gnu::packed]] Settlement // 0xB9
{
    uint32_t TimeOffset;
    uint8_t Symbol[6];
    uint32_t TradeDate;
    int64_t SettlementPrice;
    uint8_t Issue;
};
static_assert(sizeof(Settlement) == 23, "Settlement struct must be 23 bytes");

struct [[gnu::packed]] OpenInterest // 0xD3
{
    uint32_t TimeOffset;
    uint8_t Symbol[6];
    uint32_t TradeDate;
    uint32_t OpenInterest;
};
static_assert(sizeof(OpenInterest) == 18, "OpenInterest struct must be 18 bytes");

struct [[gnu::packed]] EndOfDaySummary // 0xBA
{
    uint32_t TimeOffset;
    uint8_t Symbol[6];
    uint32_t TradeDate;
    uint32_t OpenInterest;
    int64_t HighPrice;
    int64_t LowPrice;
    int64_t OpenPrice;
    int64_t ClosePrice;
    uint32_t TotalVolume;
    uint32_t BlockVolume;
    uint32_t EcrpVolume;
    uint8_t SummaryFlags;
};
static_assert(sizeof(EndOfDaySummary) == 63, "EndOfDaySummary struct must be 18 bytes");

struct [[gnu::packed]] TradingStatus // 0x31
{
    uint32_t TimeOffset;
    char Symbol[6]; // uint8_t to char
    uint16_t Reserved;
    uint8_t TradingStatus;
    uint8_t Reserved2[3];
};
static_assert(sizeof(TradingStatus) == 16, "TradingStatus struct must be 16 bytes");

struct [[gnu::packed]] EndOfSession // 0x2D
{
    uint32_t Timestamp;
};
static_assert(sizeof(EndOfSession) == 4, "EndOfSession struct must be 4 bytes");