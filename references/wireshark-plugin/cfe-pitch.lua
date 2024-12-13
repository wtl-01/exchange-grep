local debug_level = {
   DISABLED = 0,
   LEVEL_1  = 1,
   LEVEL_2  = 2
}

local default_settings = {
   debug_level  = debug_level.LEVEL_1,
   --
   -- whether this dissector is enabled or not
   enabled      = true,
   --
   -- MCast Groups 
   mcast_groups = {
      [ 1] = { name = "UDP port  #1", description = "UDP port  #1", port = 30001 },
      [ 2] = { name = "UDP port  #2", description = "UDP port  #2", port = 30002 },
      [ 3] = { name = "UDP port  #3", description = "UDP port  #3", port = 30101 },
      [ 4] = { name = "UDP port  #4", description = "UDP port  #4", port = 30102 },
      [ 5] = { name = "UDP port  #5", description = "UDP port  #5", port = 32001 },
      [ 6] = { name = "UDP port  #6", description = "UDP port  #6", port = 32002 },
      [ 7] = { name = "UDP port  #7", description = "UDP port  #7", port = 32101 },
      [ 8] = { name = "UDP port  #8", description = "UDP port  #8", port = 32102 },
      [ 9] = { name = "UDP port  #9", description = "UDP port  #9", port = 0 },
      [10] = { name = "UDP port #10", description = "UDP port #10", port = 0 },
      [11] = { name = "UDP port #11", description = "UDP port #11", port = 0 },
      [12] = { name = "UDP port #12", description = "UDP port #12", port = 0 },
      [13] = { name = "UDP port #13", description = "UDP port #13", port = 0 },
      [14] = { name = "UDP port #14", description = "UDP port #14", port = 0 },
      [15] = { name = "UDP port #15", description = "UDP port #15", port = 0 },
      [16] = { name = "UDP port #16", description = "UDP port #16", port = 0 },
   }
}

p_CFE_MCAST_PITCH = Proto("cfe-mcast-pitch", "CFE Multicast Depth of Book (PITCH) Protocol")

local dprint = function()
end

local dprint2 = function()
end

local resetDebugLevel = function()
   --info(string.format("Output Level: [%s]", p_CFE_MCAST_PITCH.prefs.debug))

   if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.debug ~= nil then
      if p_CFE_MCAST_PITCH.prefs.debug > debug_level.DISABLED then
         dprint = function(...)
            -- info(table.concat({"Lua: ", ...}," "))
         end

         if p_CFE_MCAST_PITCH.prefs.debug > debug_level.LEVEL_1 then
            dprint2 = dprint
         end
      else
         dprint = function() end
         dprint2 = dprint
      end
   else
      if default_settings.debug_level > debug_level.DISABLED then
         dprint = function(...)
            -- info(table.concat({"Lua: ", ...}," "))
         end

         if default_settings.debug_level > debug_level.LEVEL_1 then
            dprint2 = dprint
         end
      else
         dprint = function() end
         dprint2 = dprint
      end
   end
end

local cfepitchmdp = {
   protos = {
      p_SequencedUnitHeader = Proto("cfe-mcast-pitch.sequenced-unit-header", "Sequenced Unit Header", "Sequenced Unit Header"),
      p_TimeMessage = Proto("cfe-mcast-pitch.time-message", "Time", "CFE PITCH Time message"),
      p_UnitClearMessage = Proto("cfe-mcast-pitch.unit-clear-message", "Unit Clear", "CFE PITCH Unit Clear message"),
      p_TimeReferenceMessage = Proto("cfe-mcast-pitch.time-reference-message", "Time Reference", "CFE PITCH Time Reference message"),
      p_FuturesInstrumentDefinitionMessage = Proto("cfe-mcast-pitch.futures-instrument-definition-message", "Futures Instrument Definition", "CFE PITCH Futures Instrument Definition message"),
      p_PriceLimitsMessage = Proto("cfe-mcast-pitch.price-limits-message", "Price Limits", "CFE PITCH Price Limits message"),
      p_AddOrderMessage = Proto("cfe-mcast-pitch.add-order-message", "Add Order", "CFE PITCH Add Order message"),
      p_OrderExecutedMessage = Proto("cfe-mcast-pitch.order-executed-message", "Order Executed", "CFE PITCH Order Executed message"),
      p_OrderExecutedAtPriceSizeMessage = Proto("cfe-mcast-pitch.order-executed-at-price-size-message", "Order Executed at Price/Size", "CFE PITCH Order Executed at Price/Size message"),
      p_ReduceSizeMessage = Proto("cfe-mcast-pitch.reduce-size-message", "Reduce Size", "CFE PITCH Reduce Size message"),
      p_ModifyOrderMessage = Proto("cfe-mcast-pitch.modify-order-message", "Modify Order", "CFE PITCH Modify Order message"),
      p_DeleteOrderMessage = Proto("cfe-mcast-pitch.delete-order-message", "Delete Order", "CFE PITCH Delete Order message"),
      p_TradeMessage = Proto("cfe-mcast-pitch.trade-message", "Trade", "CFE PITCH Trade message"),
      p_TransactionBeginMessage = Proto("cfe-mcast-pitch.transaction-begin-message", "Transaction Begin", "CFE PITCH Transaction Begin message"),
      p_TransactionEndMessage = Proto("cfe-mcast-pitch.transaction-end-message", "Transaction End", "CFE PITCH Transaction End message"),
      p_TradeBreakMessage = Proto("cfe-mcast-pitch.trade-break-message", "Trade Break", "CFE PITCH Trade Break message"),
      p_SettlementMessage = Proto("cfe-mcast-pitch.settlement-message", "CFE PITCH Settlement message"),
      p_EndOfDaySummaryMessage = Proto("cfe-mcast-pitch.end-of-day-summary-message", "End of Day Summary", "CFE PITCH End of Day Summary message"),
      p_TradingStatusMessage = Proto("cfe-mcast-pitch.trading-status-message", "Trading Status", "CFE PITCH Trading Status message"),
      p_EndOfSessionMessage = Proto("cfe-mcast-pitch.end-of-session-message", "End of Session", "CFE PITCH End of Session message"),
      p_UnsupportedMessage = Proto("cfe-mcast-pitch.uknown-message", "Unknown/Unsupported",  "Unknown/Unsupported"),
   },
   fields = {
      f_HdrLength = ProtoField.uint16("cfe-mcast-pitch.hdr-length", "Hdr-Length"),
      f_HdrCount = ProtoField.uint8("cfe-mcast-pitch.hdr-count", "Hdr-Count"),
      f_HdrUnit = ProtoField.uint8("cfe-mcast-pitch.hdr-unit", "Hdr-Unit"),
      f_HdrSequence = ProtoField.uint32("cfe-mcast-pitch.hdr-sequence", "Hdr-Sequence"),
      f_Length  = ProtoField.uint8("cfe-mcast-pitch.length", "Length"),
      f_MessageType = ProtoField.uint8("cfe-mcast-pitch.message-type", "Message Type", base.HEX),
      f_Time = ProtoField.uint32("cfe-mcast-pitch.time", "Time"),
      f_EpochTime = ProtoField.uint32("cfe-mcast-pitch.epoch-time", "Epoch Time"),
      f_TimeOffset = ProtoField.uint32("cfe-mcast-pitch.time-offset", "Time Offset"),
      f_Symbol = ProtoField.string("cfe-mcast-pitch.symbol", "Symbol"),
      f_UnitTimestamp = ProtoField.absolute_time("cfe-mcast-pitch.unit-timestamp", "Unit Timestamp", base.UTC),
      f_ReportSymbol = ProtoField.string("cfe-mcast-pitch.report-symbol", "Report Symbol"),
      f_FuturesFlags = ProtoField.uint8("cfe-mcast-pitch.futures-flags", "Futures Flags"),
      f_ExpirationDate = ProtoField.uint32("cfe-mcast-pitch.expiration-date", "Expiration Date"),
      f_ContractSize = ProtoField.uint16("cfe-mcast-pitch.contract-size", "Contract Size"),
      f_ListingState = ProtoField.string("cfe-mcast-pitch.listing-state", "Listing State"),
      f_PriceIncrement = ProtoField.int64("cfe-mcast-pitch.price-increment", "Price Increment"),
      f_LegCount = ProtoField.uint8("cfe-mcast-pitch.leg-count", "Leg Count"),
      f_LegOffset = ProtoField.uint8("cfe-mcast-pitch.leg-offset", "Leg Offset"),
      f_VarianceBlockOffset = ProtoField.uint8("cfe-mcast-pitch.variance-block-offset", "Variance Block Offset"),
      f_RealizedVariance = ProtoField.int64("cfe-mcast-pitch.realized-variance", "Realized Variance"),
      f_NumExpectedPrices = ProtoField.uint16("cfe-mcast-pitch.num-expected-prices", "Num Expected Prices"),
      f_NumElapsedReturns = ProtoField.uint16("cfe-mcast-pitch.num-elapsed-returns", "Num Elapsed Returns"),
      f_PreviousSettlement = ProtoField.int64("cfe-mcast-pitch.previous-settlement", "Previous Settlement"),
      f_DiscountFactor = ProtoField.int64("cfe-mcast-pitch.discount-factor", "Discount Factor"),
      f_InitialStrike = ProtoField.int64("cfe-mcast-pitch.initial-strike", "Initial Strike"),
      f_PreviousARMVM = ProtoField.int64("cfe-mcast-pitch.previous-armvm", "Previous ARMVM"),
      f_FedFundsRate = ProtoField.int64("cfe-mcast-pitch.fed-funds-rate", "Fed Funds Rate"),
      f_LegRatio = ProtoField.int32("cfe-mcast-pitch.leg-ratio", "Leg Ratio"),
      f_LegSymbol = ProtoField.string("cfe-mcast-pitch.leg-symbol", "Leg Symbol"),
      f_UpperPriceLimit = ProtoField.int64("cfe-mcast-pitch.upper-price-limit", "Upper Price Limit"),
      f_LowerPriceLimit = ProtoField.int64("cfe-mcast-pitch.lower-price-limit", "Lower Price Limit"),
      f_OrderId = ProtoField.uint64("cfe-mcast-pitch.order-id", "Order Id"),
      f_SideIndicator = ProtoField.string("cfe-mcast-pitch.side-indicator", "Side Indicator"),
      f_Quantity = ProtoField.uint32("cfe-mcast-pitch.quantity", "Quantity"),
      f_Price = ProtoField.int64("cfe-mcast-pitch.price", "Price"),
      f_ExecutedQuantity = ProtoField.uint32("cfe-mcast-pitch.executed-quantity", "Executed Quantity"),
      f_ExecutionId = ProtoField.uint64("cfe-mcast-pitch.execution-id", "Execution Id"),
      f_TradeCondition = ProtoField.string("cfe-mcast-pitch.trade-condition", "Trade Condition"),
      f_EpochTime = ProtoField.absolute_time("cfe-mcast-pitch.epoch-time", "Epoch Time", base.UTC),
      f_MidnightReference = ProtoField.absolute_time("cfe-mcast-pitch.midnight-reference", "Midnight Reference", base.UTC),
      f_TradeDate = ProtoField.uint32("cfe-mcast-pitch.trade-date", "Trade Date"),
      f_RemainingQuantity = ProtoField.uint32("cfe-mcast-pitch.remaining-quantity", "Remaining Quantity"),
      f_CanceledQuantity = ProtoField.uint32("cfe-mcast-pitch.canceled-quantity", "Canceled Quantity"),

      f_SettlementPrice = ProtoField.int64("cfe-mcast-pitch.settlement-price", "Settlement Price"),
      f_Issue = ProtoField.string("cfe-mcast-pitch.issue", "Issue"),
      f_OpenInterest = ProtoField.int32("cfe-mcast-pitch.open-interest", "Open Interest"),
      f_HighPrice = ProtoField.int64("cfe-mcast-pitch.high-price", "High Price"),
      f_LowPrice = ProtoField.int64("cfe-mcast-pitch.low-price", "Low Price"),
      f_OpenPrice = ProtoField.int64("cfe-mcast-pitch.open-price", "Open Price"),
      f_ClosePrice = ProtoField.int64("cfe-mcast-pitch.close-price", "Close Price"),
      f_TotalVolume = ProtoField.uint32("cfe-mcast-pitch.total-volume", "Total Volume"),
      f_Reserved = ProtoField.string("cfe-mcast-pitch.reserved", "Reserved"),
      f_TradingStatus = ProtoField.string("cfe-mcast-pitch.trading-status", "Trading Status"),
   },
   consts = {
      CFE_MCAST_PITCH_SEQUENCE_UNIT_HEADER_LENGTH = 8,
      info = {
         version = "0.1.2",
         author = "Victor Kovalevich",
         description = "CFE Multicast Depth of Book (PITCH) Protocol dissector",
      },
      msg_types = {
         Time                        = 0x20,
         UnitClear                   = 0x97,
         TimeReference               = 0xB1,
         FuturesInstrumentDefinition = 0xBB,
         TimeReference               = 0xB1,
         PriceLimits                 = 0xBE,
         AddOrderLong                = 0x21,
         AddOrderShort               = 0x22,
         OrderExecuted               = 0x23,
         OrderExecutedAtPriceSize    = 0x24,
         ReduceSizeLong              = 0x25,
         ReduceSizeShort             = 0x26,
         ModifyLong                  = 0x27,
         ModifyShort                 = 0x28,
         DeleteOrder                 = 0x29,
         TradeLong                   = 0x2A,
         TradeShort                  = 0x2B,
         TransactionBegin            = 0xBC,
         TransactionEnd              = 0xBD,
         TradeBreak                  = 0x2C,
         Settlement                  = 0xB9,
         EndOfDaySummary             = 0xBA,
         TradingStatus               = 0x31,
         EndOfSession                = 0x2D,
         
      },
      msg_type_to_string = {
         [0x20] = "Time",
         [0x97] = "Unit Clear",
         [0xB1] = "Time Reference",
         [0xBB] = "Futures Instrument Definition",
         [0xB1] = "Time Reference",
         [0xBE] = "Price Limits",
         [0x21] = "Add Order",
         [0x22] = "Add Order (short)",
         [0x23] = "Order Executed",
         [0x24] = "Order Executed at Price/Size",
         [0x25] = "Reduce Size (long)",
         [0x26] = "Reduce Size (short)",
         [0x27] = "Modify (long)",
         [0x28] = "Modify (short)",
         [0x29] = "Delete Order",
         [0x2A] = "Trade (long)",
         [0x2B] = "Trade (short)",
         [0xBC] = "Transaction Begin",
         [0xBD] = "Transaction End",
         [0x2C] = "Trade Break",
         [0xB9] = "Settlement",
         [0xBA] = "End of Day Summary",
         [0x31] = "Trading Status",
         [0x2D] = "End of Session",
      },
      listing_state_to_string = {
         ["A"] = "Active",
         ["I"] = "Inactive",
         ["T"] = "Test",
      },
      trade_condition_to_string = {
         [" "] = "Normal trade",
         ["O"] = "Opening trade",
         ["S"] = "Spread trade",
         ["B"] = "Block trade",
         ["E"] = "ECRP trade",
      },
      side_indicator_to_string = {
         ["B"] = "Buy Order",
         ["S"] = "Sell Order",
      },
      issue_to_string = {
         ["S"] = "Initial Settlement",
         ["R"] = "Re-issued Settlement",
      },
      trading_status_to_string = {
         ["S"] = "Suspended",
         ["Q"] = "Queuing",
         ["T"] = "Trading",
         ["H"] = "Halted",
      },
   },
   funcs = {
      addTimeMessage = nil,
      addUnitClearMessage = nil,
      addTimeReferenceMessage = nil,
      addFuturesInstrumentDefinitionMessage = nil,
      addPriceLimitsMessage = nil,
      addAddOrderLongMessage = nil,
      addAddOrderShortMessage = nil,
      addOrderExecutedMessage = nil,
      addOrderExecutedAtPriceSizeMessage = nil,
      addReduceSizeLongMessage = nil,
      addReduceSizeShortMessage = nil,
      addModifyOrderLongMessage = nil,
      addModifyOrderShortMessage = nil,
      addDeleteOrderMessage = nil,
      addTradeLongMessage = nil,
      addTradeShortMessage = nil,
      addTransactionBeginMessage = nil,
      addTransactionEndMessage = nil,
      addTradeBreakMessage = nil,
      addSettlementMessage = nil,
      addEndOfDaySummaryMessage = nil,
      addTradingStatusMessage = nil,
      addEndOfSessionMessage = nil,
      addUnsupportedMessage = nil,
      getMessageHandler = nil,
      getMessageType = nil,
      getListingState = nil,
      getTradeCondition = nil,
      getSideIndicator = nil,
      getIssue = nil,
      getTradingStatus = nil,
      enableDissector = nil,
      disableDissector = nil,
      initMessageHandlers = nil,
   },
   runtime = {
      messageHandlers = {
      }
   }
}

p_CFE_MCAST_PITCH.fields = {
   cfepitchmdp.fields.f_HdrLength,
   cfepitchmdp.fields.f_HdrCount,
   cfepitchmdp.fields.f_HdrUnit,
   cfepitchmdp.fields.f_HdrSequence,
   cfepitchmdp.fields.f_Length,
   cfepitchmdp.fields.f_MessageType,
   cfepitchmdp.fields.f_Time,
   cfepitchmdp.fields.f_EpochTime,
   cfepitchmdp.fields.f_TimeOffset,
   cfepitchmdp.fields.f_Symbol,
   cfepitchmdp.fields.f_UnitTimestamp,
   cfepitchmdp.fields.f_ReportSymbol,
   cfepitchmdp.fields.f_FuturesFlags,
   cfepitchmdp.fields.f_ExpirationDate,
   cfepitchmdp.fields.f_ContractSize,
   cfepitchmdp.fields.f_ListingState,
   cfepitchmdp.fields.f_PriceIncrement,
   cfepitchmdp.fields.f_LegCount,
   cfepitchmdp.fields.f_LegOffset,
   cfepitchmdp.fields.f_VarianceBlockOffset,
   cfepitchmdp.fields.f_RealizedVariance,
   cfepitchmdp.fields.f_NumExpectedPrices,
   cfepitchmdp.fields.f_NumElapsedReturns,
   cfepitchmdp.fields.f_PreviousSettlement,
   cfepitchmdp.fields.f_DiscountFactor,
   cfepitchmdp.fields.f_InitialStrike,
   cfepitchmdp.fields.f_PreviousARMVM,
   cfepitchmdp.fields.f_FedFundsRate,
   cfepitchmdp.fields.f_LegRatio,
   cfepitchmdp.fields.f_LegSymbol,
   cfepitchmdp.fields.f_UpperPriceLimit,
   cfepitchmdp.fields.f_LowerPriceLimit,
   cfepitchmdp.fields.f_OrderId,
   cfepitchmdp.fields.f_SideIndicator,
   cfepitchmdp.fields.f_Quantity,
   cfepitchmdp.fields.f_Price,
   cfepitchmdp.fields.f_ExecutedQuantity,
   cfepitchmdp.fields.f_ExecutionId,
   cfepitchmdp.fields.f_TradeCondition,
   cfepitchmdp.fields.f_MidnightReference,
   cfepitchmdp.fields.f_TradeDate,
   cfepitchmdp.fields.f_RemainingQuantity,
   cfepitchmdp.fields.f_CanceledQuantity,
   cfepitchmdp.fields.f_SettlementPrice,
   cfepitchmdp.fields.f_Issue,
   cfepitchmdp.fields.f_OpenInterest,
   cfepitchmdp.fields.f_HighPrice,
   cfepitchmdp.fields.f_LowPrice,
   cfepitchmdp.fields.f_OpenPrice,
   cfepitchmdp.fields.f_ClosePrice,
   cfepitchmdp.fields.f_TotalVolume,
   cfepitchmdp.fields.f_Reserved,
   cfepitchmdp.fields.f_TradingStatus,
}

dprint2("CFE PITCH: proto fields have been registered")
set_plugin_info(cfepitchmdp.consts.info)

local bit_lib = bit32

--
-- This function will be invoked by Wireshark during initialization, such as
-- at program start and loading a new file
p_CFE_MCAST_PITCH.init = function()
   if bit_lib==nil then
      bit_lib=require("bit32")
   end
   cfepitchmdp.funcs.initMessageHandlers()
end

--
-- The following creates the callback function for the dissector.
-- It's the same as doing "p_CFE_MCAST_PITCH.dissector = function (buff, pkt, root)"
-- The 'buff' is a Tvb object, 'pktinfo' is a Pinfo object, and 'root' is a TreeItem object.
-- Whenever Wireshark dissects a packet that our Proto is hooked into, it will call
-- this function and pass it these arguments for the packet it's dissecting.
p_CFE_MCAST_PITCH.dissector = function(buff, pktinfo, root)
   --- dprint2("p_CFE_MCAST_PITCH.dissector called")

   if buff:len() < cfepitchmdp.consts.CFE_MCAST_PITCH_SEQUENCE_UNIT_HEADER_LENGTH then
      return
   end

   --
   -- We have deal with UDP protocol thus one UDP packet is equal to one CFE MCAST PITCH packet

   --
   -- Add a packet instance
   tree = root:add(p_CFE_MCAST_PITCH, buff(0))

   --
   -- Set up protocol type
   pktinfo.cols.protocol:set(p_CFE_MCAST_PITCH.name)

   --
   -- Add a packet header
   packet_header = tree:add(cfepitchmdp.protos.p_SequencedUnitHeader, buff(0, cfepitchmdp.consts.CFE_MCAST_PITCH_SEQUENCE_UNIT_HEADER_LENGTH))

   local hdrLength = buff(0, 2):le_uint()
   local hdrCount = buff(2, 1):le_uint()
   local hdrUnit = buff(3, 1):le_uint()
   local hdrSequence = buff(4, 4):le_uint()

   packet_header:add(cfepitchmdp.fields.f_HdrLength, buff(0, 2), hdrLength)
   packet_header:add(cfepitchmdp.fields.f_HdrCount, buff(2, 1), hdrCount)
   packet_header:add(cfepitchmdp.fields.f_HdrUnit, buff(3, 1), hdrUnit)
   packet_header:add(cfepitchmdp.fields.f_HdrSequence, buff(4, 4), hdrSequence)

   local offset = cfepitchmdp.consts.CFE_MCAST_PITCH_SEQUENCE_UNIT_HEADER_LENGTH

   if hdrCount==0 then
      tree:append_text(" [Heartbeat message]")
   else
      for i = 1, hdrCount, 1 do
         local msgLength = buff(offset+0, 1):le_uint()
         local msgType = buff(offset+1, 1):le_uint()
         local msgHandler = cfepitchmdp.funcs.getMessageHandler(msgType)
         msgHandler(buff(offset, msgLength), pktinfo, tree, i)
         offset = offset + msgLength
      end
   end
end

cfepitchmdp.funcs.enableDissector = function()
   -- call it now
   resetDebugLevel()

   dprint("Turn on CFE MCAST PITCH dissector")

   --
   -- Register a chained dissector
   local udp_table = DissectorTable.get("udp.port")

   if p_CFE_MCAST_PITCH.prefs ~= nil then
      if p_CFE_MCAST_PITCH.prefs.udp_port_1 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_1 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_1))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_1, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_2 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_2 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_2))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_2, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_3 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_3 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_3))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_3, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_4 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_4 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_4))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_4, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_5 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_5 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_5))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_5, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_6 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_6 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_6))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_6, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_7 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_7 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_7))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_7, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_8 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_8 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_8))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_8, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_9 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_9 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_9))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_9, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_10 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_10 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_10))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_10, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_11 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_11 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_11))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_11, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_12 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_12 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_12))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_12, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_13 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_13 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_13))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_13, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_14 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_14 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_14))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_14, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_15 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_15 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_15))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_15, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_16 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_16 ~=0 then
         dprint(string.format("Added udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_16))
         udp_table:add(p_CFE_MCAST_PITCH.prefs.udp_port_16, p_CFE_MCAST_PITCH)
      end
   end
end

cfepitchmdp.funcs.disableDissector = function()
   dprint("Turn off CFE MCAST PITCH dissector");

   local udp_table = DissectorTable.get("udp.port")

   if p_CFE_MCAST_PITCH.prefs ~= nil then
      if p_CFE_MCAST_PITCH.prefs.udp_port_1 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_1 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_1))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_1, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_2 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_2 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_2))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_2, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_3 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_3 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_3))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_3, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_4 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_4 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_4))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_4, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_5 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_5 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_5))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_5, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_6 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_6 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_6))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_6, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_7 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_7 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_7))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_7, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_8 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_8 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_8))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_8, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_9 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_9 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_9))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_9, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_10 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_10 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_10))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_10, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_11 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_11 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_11))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_11, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_12 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_12 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_12))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_12, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_13 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_13 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_13))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_13, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_14 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_14 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_14))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_14, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_15 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_15 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_15))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_15, p_CFE_MCAST_PITCH)
      end
      if p_CFE_MCAST_PITCH.prefs.udp_port_16 ~=nil and p_CFE_MCAST_PITCH.prefs.udp_port_16 ~=0 then
         dprint(string.format("Removed udp port [%u]", p_CFE_MCAST_PITCH.prefs.udp_port_16))
         udp_table:remove(p_CFE_MCAST_PITCH.prefs.udp_port_16, p_CFE_MCAST_PITCH)
      end
   end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_1_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_1_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_1_ip_port, p_CFE_MCAST_PITCH)
   -- end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_2_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_2_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_2_ip_port, p_CFE_MCAST_PITCH)
   -- end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_3_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_3_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_3_ip_port, p_CFE_MCAST_PITCH)
   -- end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_4_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_4_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_4_ip_port, p_CFE_MCAST_PITCH)
   -- end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_5_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_5_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_1_ip_port, p_CFE_MCAST_PITCH)
   -- end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_2_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_2_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_2_ip_port, p_CFE_MCAST_PITCH)
   -- end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_3_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_3_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_3_ip_port, p_CFE_MCAST_PITCH)
   -- end

   -- if p_CFE_MCAST_PITCH.prefs ~= nil and p_CFE_MCAST_PITCH.prefs.group_4_ip_port then
   --    dprint2(string.format("CFE MCAST PITCH dissector: removing UDP port [%u]",
   --                          p_CFE_MCAST_PITCH.prefs.group_4_ip_port))
   --    udp_table:remove(p_CFE_MCAST_PITCH.prefs.group_4_ip_port, p_CFE_MCAST_PITCH)
   -- end
   
end

cfepitchmdp.funcs.addTimeMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TimeMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local epochTime = buff(6, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]",
                                                                                                cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_EpochTime, buff(6, 4), NSTime(epochTime)):append_text(string.format(" [%u]", epochTime))
end

cfepitchmdp.funcs.addUnitClearMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_UnitClearMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
end

cfepitchmdp.funcs.addTimeReferenceMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TimeReferenceMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local midnightReference = buff(2, 4):le_uint()
   local time = buff(6, 4):le_uint()
   local timeOffset = buff(10, 4):le_uint()
   local tradeDate = buff(14, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_MidnightReference, buff(2, 4), NSTime(midnightReference)):append_text(string.format(" [%u]", midnightReference))
   msgTree:add(cfepitchmdp.fields.f_Time, buff(6, 4), time)
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(10, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_TradeDate, buff(14, 4), tradeDate)
end

cfepitchmdp.funcs.addFuturesInstrumentDefinitionMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_FuturesInstrumentDefinitionMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local unitTimestamp = buff(12, 4):le_uint()
   local futuresFlags =  buff(22, 1):le_uint()
   local expirationDate = buff(23, 4):le_uint()
   local contractSize = buff(27, 2):le_uint()
   local listingState = buff(29, 1):string()
   local priceIncrement = buff(30, 8):le_int64()
   local legCount = buff(38, 1):le_uint()
   local legOffset = buff(39, 1):le_uint()
   local varianceBlockOffset = buff(40, 1):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(6, 6))
   msgTree:add(cfepitchmdp.fields.f_UnitTimestamp, buff(12, 4), NSTime(unitTimestamp)):append_text(string.format(" [%u]", unitTimestamp))
   msgTree:add(cfepitchmdp.fields.f_ReportSymbol, buff(16, 6))
   msgTree:add(cfepitchmdp.fields.f_FuturesFlags, buff(22, 1), futuresFlags)
   msgTree:add(cfepitchmdp.fields.f_ExpirationDate, buff(23, 4), expirationDate)
   msgTree:add(cfepitchmdp.fields.f_ContractSize, buff(27, 2), contractSize)
   msgTree:add(cfepitchmdp.fields.f_ListingState, buff(29, 1), listingState):append_text(string.format(" [%s]", cfepitchmdp.funcs.getListingState(listingState)))
   msgTree:add(cfepitchmdp.fields.f_PriceIncrement, buff(30, 8), priceIncrement)
   msgTree:add(cfepitchmdp.fields.f_LegCount, buff(38, 1), legCount)
   msgTree:add(cfepitchmdp.fields.f_LegOffset, buff(39, 1), legOffset)
   msgTree:add(cfepitchmdp.fields.f_VarianceBlockOffset, buff(40, 1), varianceBlockOffset)
   local offset = legOffset
   if bit_lib.band(futuresFlags, 0x01)==0x01 then
      local realizedVariance = buff(41, 8):le_int64()
      local numExpectedPrices = buff(49, 2):le_uint()
      local numElapsedReturns = buff(51, 2):le_uint()
      local previousSettlement = buff(53, 8):le_int64()
      local discountFactor = buff(61, 8):le_int64()
      local initialStrike = buff(69, 8):le_int64()
      local previousARMVM = buff(77, 8):le_int64()
      local fedFundsRate = buff(85, 8):le_int64()
      offset=offset+52
      msgTree:add(cfepitchmdp.fields.f_RealizedVariance, buff(41, 8), realizedVariance)
      msgTree:add(cfepitchmdp.fields.f_NumExpectedPrices, buff(49, 2), numExpectedPrices)
      msgTree:add(cfepitchmdp.fields.f_NumElapsedReturns, buff(51, 2), numElapsedReturns)
      msgTree:add(cfepitchmdp.fields.f_PreviousSettlement, buff(53, 8), previousSettlement)
      msgTree:add(cfepitchmdp.fields.f_DiscountFactor, buff(61, 8), discountFactor)
      msgTree:add(cfepitchmdp.fields.f_InitialStrike, buff(69, 8), initialStrike)
      msgTree:add(cfepitchmdp.fields.f_PreviousARMVM, buff(77, 8), previousARMVM)
      msgTree:add(cfepitchmdp.fields.f_FedFundsRate, buff(85, 8), fedFundsRate)
   elseif bit_lib.band(futuresFlags, 0x01)==0x00 then
   end
   for i = 1, legCount, 1 do
      local legRatio = buff(offset, 4):le_int()
      msgTree:add(cfepitchmdp.fields.f_LegRatio, buff(offset, 4), legRatio)
      msgTree:add(cfepitchmdp.fields.f_LegSymbol, buff(offset+4, 6))
      offset=offset+10
   end
end

cfepitchmdp.funcs.addPriceLimitsMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_PriceLimitsMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local upperPriceLimit = buff(12, 8):le_int64()
   local lowerPriceLimit = buff(20, 8):le_int64()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(6, 6))
   msgTree:add(cfepitchmdp.fields.f_UpperPriceLimit, buff(12, 8), upperPriceLimit)
   msgTree:add(cfepitchmdp.fields.f_LowerPriceLimit, buff(20, 8), lowerPriceLimit)
end

cfepitchmdp.funcs.addAddOrderLongMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_AddOrderMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local sideIndicator = buff(14, 1):string()
   local quantity = buff(15, 4):le_uint()
   local price = buff(25, 8):le_int64()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_SideIndicator, buff(14, 1), sideIndicator):append_text(string.format(" [%s]", cfepitchmdp.funcs.getSideIndicator(sideIndicator)))
   msgTree:add(cfepitchmdp.fields.f_Quantity, buff(15, 4), quantity)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(19, 6))
   msgTree:add(cfepitchmdp.fields.f_Price, buff(25, 8), price):append_text(string.format(" [%.5f]", price:tonumber()/10000))
end

cfepitchmdp.funcs.addAddOrderShortMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_AddOrderMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local sideIndicator = buff(14, 1):string()
   local quantity = buff(15, 2):le_uint()
   local price = buff(23, 2):le_int64()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_SideIndicator, buff(14, 1), sideIndicator):append_text(string.format(" [%s]", cfepitchmdp.funcs.getSideIndicator(sideIndicator)))
   msgTree:add(cfepitchmdp.fields.f_Quantity, buff(15, 2), quantity)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(17, 6))
   msgTree:add(cfepitchmdp.fields.f_Price, buff(23, 2), Int64(price)):append_text(string.format(" [%.5f]", price:tonumber()/100))
end

cfepitchmdp.funcs.addOrderExecutedMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_OrderExecutedMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local executedQuantity = buff(14, 4):le_uint()
   local executionId = buff(18, 8):le_uint64()
   local tradeCondition = buff(26, 1):string()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]",cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_ExecutedQuantity, buff(14, 4), executedQuantity)
   msgTree:add(cfepitchmdp.fields.f_ExecutionId, buff(18, 8), executionId)
   msgTree:add(cfepitchmdp.fields.f_TradeCondition, buff(26, 1), tradeCondition):append_text(string.format(" [%s]", cfepitchmdp.funcs.getTradeCondition(tradeCondition)))
end

cfepitchmdp.funcs.addOrderExecutedAtPriceSizeMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_OrderExecutedAtPriceSizeMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local executedQuantity = buff(14, 4):le_uint()
   local remainingQuantity = buff(18, 4):le_uint()
   local executionId = buff(22, 8):le_uint64()
   local tradeCondition = buff(30, 1):string()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_ExecutedQuantity, buff(14, 4), executedQuantity)
   msgTree:add(cfepitchmdp.fields.f_RemainingQuantity, buff(18, 4), remainingQuantity)
   msgTree:add(cfepitchmdp.fields.f_ExecutionId, buff(18, 8), executionId)
   msgTree:add(cfepitchmdp.fields.f_TradeCondition, buff(26, 1), tradeCondition):append_text(string.format(" [%s]", cfepitchmdp.funcs.getTradeCondition(tradeCondition)))
end

cfepitchmdp.funcs.addReduceSizeLongMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_ReduceSizeMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local canceledQuantity = buff(14, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_CanceledQuantity, buff(14, 4), canceledQuantity)
end

cfepitchmdp.funcs.addReduceSizeShortMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_ReduceSizeMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local canceledQuantity = buff(14, 2):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_CanceledQuantity, buff(14, 2), canceledQuantity)
end

cfepitchmdp.funcs.addModifyOrderLongMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_ModifyOrderMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local quantity = buff(14, 4):le_uint()
   local price = buff(18, 8):le_int64()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_Quantity, buff(14, 4), quantity)
   msgTree:add(cfepitchmdp.fields.f_Price, buff(18, 8), price):append_text(string.format(" [%.5f]", price:tonumber()/10000))
end

cfepitchmdp.funcs.addModifyOrderShortMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_ModifyOrderMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local quantity = buff(14, 2):le_uint()
   local price = buff(16, 2):le_int64()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_Quantity, buff(14, 2), quantity)
   msgTree:add(cfepitchmdp.fields.f_Price, buff(16, 2), price):append_text(string.format(" [%.5f]", price:tonumber()/100))
end

cfepitchmdp.funcs.addDeleteOrderMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_DeleteOrderMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
end

cfepitchmdp.funcs.addTradeLongMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TradeMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local sideIndicator = buff(14, 1):string()
   local quantity = buff(15, 4):le_uint()
   local price = buff(25, 8):le_int64()
   local executionId = buff(33, 8):le_uint64()
   local tradeCondition = buff(41, 1):string()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_SideIndicator, buff(14, 1), sideIndicator):append_text(string.format(" [%s]", cfepitchmdp.funcs.getSideIndicator(sideIndicator)))
   msgTree:add(cfepitchmdp.fields.f_Quantity, buff(15, 4), quantity)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(19, 6))
   msgTree:add(cfepitchmdp.fields.f_Price, buff(25, 8), price):append_text(string.format(" [%.5f]", price:tonumber()/10000))
   msgTree:add(cfepitchmdp.fields.f_ExecutionId, buff(33, 8), executionId)
   msgTree:add(cfepitchmdp.fields.f_TradeCondition, buff(41, 1), tradeCondition):append_text(string.format(" [%s]", cfepitchmdp.funcs.getTradeCondition(tradeCondition)))
end

cfepitchmdp.funcs.addTradeShortMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TradeMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local orderId = buff(6, 8):le_uint64()
   local sideIndicator = buff(14, 1):string()
   local quantity = buff(15, 2):le_uint()
   local price = buff(23, 2):le_int64()
   local executionId = buff(25, 8):le_uint64()
   local tradeCondition = buff(33, 1):string()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_OrderId, buff(6, 8), orderId)
   msgTree:add(cfepitchmdp.fields.f_SideIndicator, buff(14, 1), sideIndicator):append_text(string.format(" [%s]", cfepitchmdp.funcs.getSideIndicator(sideIndicator)))
   msgTree:add(cfepitchmdp.fields.f_Quantity, buff(15, 2), quantity)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(17, 6))
   msgTree:add(cfepitchmdp.fields.f_Price, buff(23, 2), price):append_text(string.format(" [%.5f]", price:tonumber()/100))
   msgTree:add(cfepitchmdp.fields.f_ExecutionId, buff(25, 8), executionId)
   msgTree:add(cfepitchmdp.fields.f_TradeCondition, buff(33, 1), tradeCondition):append_text(string.format(" [%s]", cfepitchmdp.funcs.getTradeCondition(tradeCondition)))
end

cfepitchmdp.funcs.addTransactionBeginMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TransactionBeginMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
end

cfepitchmdp.funcs.addTransactionEndMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TransactionEndMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
end

cfepitchmdp.funcs.addTradeBreakMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TradeBreakMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
end

cfepitchmdp.funcs.addSettlementMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_SettlementMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local tradeDate = buff(12, 4):le_uint()
   local settlementPrice = buff(16, 8):le_int64()
   local issue = buff(24, 1):string()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(6, 6))
   msgTree:add(cfepitchmdp.fields.f_TradeDate, buff(12, 4), tradeDate)
   msgTree:add(cfepitchmdp.fields.f_SettlementPrice, buff(16, 8), price):append_text(string.format(" [%.5f]", settlementPrice:tonumber()/10000))
   msgTree:add(cfepitchmdp.fields.f_Issue, buff(24, 1), issue):append_text(string.format(" [%s]", cfepitchmdp.funcs.getIssue(issue)))
end

cfepitchmdp.funcs.addEndOfDaySummaryMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_EndOfDaySummaryMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local tradeDate = buff(12, 4):le_uint()
   local openInterest = buff(16, 4):le_int()
   local highPrice = buff(20, 8):le_int64()
   local lowPrice = buff(28, 8):le_int64()
   local openPrice = buff(36, 8):le_int64()
   local closePrice = buff(44, 8):le_int64()
   local totalVolume = buff(52, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(6, 6))
   msgTree:add(cfepitchmdp.fields.f_TradeDate, buff(12, 4), tradeDate)
   msgTree:add(cfepitchmdp.fields.f_OpenInterest, buff(16, 4), openInterest)
   msgTree:add(cfepitchmdp.fields.f_HighPrice, buff(20, 8), highPrice):append_text(string.format(" [%.5f]", highPrice:tonumber()/10000))
   msgTree:add(cfepitchmdp.fields.f_LowPrice, buff(28, 8), lowPrice):append_text(string.format(" [%.5f]", lowPrice:tonumber()/10000))
   msgTree:add(cfepitchmdp.fields.f_OpenPrice, buff(36, 8), openPrice):append_text(string.format(" [%.5f]", openPrice:tonumber()/10000))
   msgTree:add(cfepitchmdp.fields.f_ClosePrice, buff(44, 8), closePrice):append_text(string.format(" [%.5f]", closePrice:tonumber()/10000))
   msgTree:add(cfepitchmdp.fields.f_TotalVolume, buff(52, 4), totalVolume)
end

cfepitchmdp.funcs.addTradingStatusMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_TradingStatusMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   local tradingStatus = buff(14, 1):string()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
   msgTree:add(cfepitchmdp.fields.f_Symbol, buff(6, 6))
   msgTree:add(cfepitchmdp.fields.f_Reserved, buff(12, 2))
   msgTree:add(cfepitchmdp.fields.f_TradingStatus, buff(14, 1), tradingStatus):append_text(string.format(" [%s]", cfepitchmdp.funcs.getTradingStatus(tradingStatus)))
   msgTree:add(cfepitchmdp.fields.f_Reserved, buff(15, 3))
end

cfepitchmdp.funcs.addEndOfSessionMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_EndOfSessionMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   local timeOffset = buff(2, 4):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]", cfepitchmdp.funcs.getMessageType(msgType)))
   msgTree:add(cfepitchmdp.fields.f_TimeOffset, buff(2, 4), timeOffset)
end

cfepitchmdp.funcs.addUnsupportedMessage = function(buff, pktinfo, root, idx)
   msgTree = root:add(cfepitchmdp.protos.p_UnsupportedMessage, buff):append_text(string.format(" [%u]", idx))
   local msgLength = buff(0, 1):le_uint()
   local msgType = buff(1, 1):le_uint()
   msgTree:add(cfepitchmdp.fields.f_Length, buff(0, 1), msgLength)
   msgTree:add(cfepitchmdp.fields.f_MessageType, buff(1, 1), msgType):append_text(string.format(" [%s]",
                                                                                                cfepitchmdp.funcs.getMessageType(msgType)))
end

cfepitchmdp.funcs.getMessageHandler = function(value)
   local handler = cfepitchmdp.runtime.messageHandlers[value]
   if handler==nil then
      return cfepitchmdp.funcs.addUnsupportedMessage
   end
   return handler
   
end

cfepitchmdp.funcs.getMessageType  = function(value)
   return (cfepitchmdp.consts.msg_type_to_string[value]~=nil and cfepitchmdp.consts.msg_type_to_string[value] or value)
end

cfepitchmdp.funcs.getListingState = function(value)
   return (cfepitchmdp.consts.listing_state_to_string[value]~=nil and cfepitchmdp.consts.listing_state_to_string[value] or value)
end

cfepitchmdp.funcs.getTradeCondition = function(value)
   return (cfepitchmdp.consts.trade_condition_to_string[value]~=nil and cfepitchmdp.consts.trade_condition_to_string[value] or value)
end

cfepitchmdp.funcs.getSideIndicator = function(value)
   return (cfepitchmdp.consts.side_indicator_to_string[value]~=nil and cfepitchmdp.consts.side_indicator_to_string[value] or value)
end

cfepitchmdp.funcs.getIssue = function(value)
   return (cfepitchmdp.consts.issue_to_string[value]~=nil and cfepitchmdp.consts.issue_to_string[value] or value)
end

cfepitchmdp.funcs.getTradingStatus = function(value)
   return (cfepitchmdp.consts.trading_status_to_string[value]~=nil and cfepitchmdp.consts.trading_status_to_string[value] or value)
end

cfepitchmdp.funcs.initMessageHandlers  = function()
   if #cfepitchmdp.runtime.messageHandlers~=0 then
      return
   end
   cfepitchmdp.runtime.messageHandlers = {
      [cfepitchmdp.consts.msg_types.Time]                        = cfepitchmdp.funcs.addTimeMessage,
      [cfepitchmdp.consts.msg_types.UnitClear]                   = cfepitchmdp.funcs.addUnitClearMessage,
      [cfepitchmdp.consts.msg_types.TimeReference]               = cfepitchmdp.funcs.addTimeReferenceMessage,
      [cfepitchmdp.consts.msg_types.FuturesInstrumentDefinition] = cfepitchmdp.funcs.addFuturesInstrumentDefinitionMessage,
      [cfepitchmdp.consts.msg_types.TimeReference]               = cfepitchmdp.funcs.addTimeReferenceMessage,
      [cfepitchmdp.consts.msg_types.PriceLimits]                 = cfepitchmdp.funcs.addPriceLimitsMessage,
      [cfepitchmdp.consts.msg_types.AddOrderLong]                = cfepitchmdp.funcs.addAddOrderLongMessage,
      [cfepitchmdp.consts.msg_types.AddOrderShort]               = cfepitchmdp.funcs.addAddOrderShortMessage,
      [cfepitchmdp.consts.msg_types.OrderExecuted]               = cfepitchmdp.funcs.addOrderExecutedMessage,
      [cfepitchmdp.consts.msg_types.OrderExecutedAtPriceSize]    = cfepitchmdp.funcs.addOrderExecutedAtPriceSizeMessage,
      [cfepitchmdp.consts.msg_types.ReduceSizeLong]              = cfepitchmdp.funcs.addReduceSizeLongMessage,
      [cfepitchmdp.consts.msg_types.ReduceSizeShort]             = cfepitchmdp.funcs.addReduceSizeShortMessage,
      [cfepitchmdp.consts.msg_types.ModifyLong]                  = cfepitchmdp.funcs.addModifyOrderLongMessage,
      [cfepitchmdp.consts.msg_types.ModifyShort]                 = cfepitchmdp.funcs.addModifyOrderShortMessage,
      [cfepitchmdp.consts.msg_types.DeleteOrder]                 = cfepitchmdp.funcs.addDeleteOrderMessage,
      [cfepitchmdp.consts.msg_types.TradeLong]                   = cfepitchmdp.funcs.addTradeLongMessage,
      [cfepitchmdp.consts.msg_types.TradeShort]                  = cfepitchmdp.funcs.addTradeShortMessage,
      [cfepitchmdp.consts.msg_types.TransactionBegin]            = cfepitchmdp.funcs.addTransactionBeginMessage,
      [cfepitchmdp.consts.msg_types.TransactionEnd]              = cfepitchmdp.funcs.addTransactionEndMessage,
      [cfepitchmdp.consts.msg_types.TradeBreak]                  = cfepitchmdp.funcs.addTradeBreakMessage,
      [cfepitchmdp.consts.msg_types.Settlement]                  = cfepitchmdp.funcs.addSettlementMessage,
      [cfepitchmdp.consts.msg_types.EndOfDaySummary]             = cfepitchmdp.funcs.addEndOfDaySummaryMessage,
      [cfepitchmdp.consts.msg_types.TradingStatus]               = cfepitchmdp.funcs.addTradingStatusMessage,
      [cfepitchmdp.consts.msg_types.EndOfSession]                = cfepitchmdp.funcs.addEndOfSessionMessage,
   }
end

local set_default_properties = function()
   dprint2(string.format("set_default_properties: [%s]", tostring(default_settings.enabled)))

   local debug_level_tab = {
      { 1, "Off",     debug_level.DISABLED },
      { 2, "Debug-1", debug_level.LEVEL_1  },
      { 3, "Debug-2", debug_level.LEVEL_2  }
   }

   --
   -- Register our preferences
   p_CFE_MCAST_PITCH.prefs.enabled = Pref.bool("Dissector enabled",
                                               default_settings.enabled,
                                               "Whether the CSM dissector is enabled or not")
   p_CFE_MCAST_PITCH.prefs.debug   = Pref.enum("Debug",
                                               default_settings.debug_level,
                                               "The debug printing level",
                                               debug_level_tab,
                                               true)
   local idx = 0
   local grps = default_settings.mcast_groups
   
   p_CFE_MCAST_PITCH.prefs.udp_port_1  = Pref.uint(grps[1].name,  grps[1].port,  grps[1].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_2  = Pref.uint(grps[2].name,  grps[2].port,  grps[2].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_3  = Pref.uint(grps[3].name,  grps[3].port,  grps[3].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_4  = Pref.uint(grps[4].name,  grps[4].port,  grps[4].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_5  = Pref.uint(grps[5].name,  grps[5].port,  grps[5].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_6  = Pref.uint(grps[6].name,  grps[6].port,  grps[6].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_7  = Pref.uint(grps[7].name,  grps[7].port,  grps[7].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_8  = Pref.uint(grps[8].name,  grps[8].port,  grps[8].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_9  = Pref.uint(grps[9].name,  grps[9].port,  grps[9].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_10 = Pref.uint(grps[10].name, grps[10].port, grps[10].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_11 = Pref.uint(grps[11].name, grps[11].port, grps[11].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_12 = Pref.uint(grps[12].name, grps[12].port, grps[12].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_13 = Pref.uint(grps[13].name, grps[13].port, grps[13].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_14 = Pref.uint(grps[14].name, grps[14].port, grps[14].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_15 = Pref.uint(grps[15].name, grps[15].port, grps[15].description)
   p_CFE_MCAST_PITCH.prefs.udp_port_16 = Pref.uint(grps[16].name, grps[16].port, grps[16].description)
end

--
-- The function for handling preferences being changed
p_CFE_MCAST_PITCH.prefs_changed = function()
   dprint2("p_CFE_MCAST_PITCH.prefs_changed")
   default_settings.enabled=p_CFE_MCAST_PITCH.prefs.enabled
   if default_settings.enabled then
      cfepitchmdp.funcs.enableDissector()
   else
      cfepitchmdp.funcs.disableDissector()
   end
   --
   -- Have to reload the capture file for this type of change
   reload()
end

set_default_properties()

-----
----- Call it now, because we're enabled by default
cfepitchmdp.funcs.enableDissector()
