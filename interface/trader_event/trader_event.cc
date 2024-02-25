/*
 * traderEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/trader_event/trader_event.h"
#include "common/extern/log/log.h"
#include "common/self/global_sem.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/utils.h"
#include "market/domain/components/instrument_info.h"
#include "market/domain/market_service.h"


TraderEvent::TraderEvent() { RegMsgFun(); }

void TraderEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map_.find(msg.msg_name);
  if (iter != msg_func_map_.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msg_name.c_str());
  return;
}

void TraderEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map_.clear();
  msg_func_map_["QryInstrumentRsp"] = [this](utils::ItpMsg &msg) { QryInstrumentRspHandle(msg); };
  msg_func_map_["MarketStateRsp"] = [this](utils::ItpMsg &msg) { MarketStateRspHandle(msg); };

  for (auto &iter : msg_func_map_) {
    INFO_LOG("msg_func_map_[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
  return;
}

void TraderEvent::QryInstrumentRspHandle(utils::ItpMsg &msg) {
  auto &market_server = MarketService::GetInstance();

  market_trader::message message;
  message.ParseFromString(msg.pb_msg);
  auto &rsp = message.qry_instrument_rsp();

  InstrumentInfo::Info instrument_info;
  instrument_info.exch = rsp.exchange_id();
  instrument_info.is_trading = rsp.is_trade();
  instrument_info.tradeuint = rsp.tradeuint();
  instrument_info.ticksize = rsp.ticksize();
  instrument_info.max_limit_order_volume = rsp.max_limit_volume();
  instrument_info.min_limit_order_volume = rsp.min_limit_volume();
  instrument_info.max_market_order_volume = rsp.max_market_volume();
  instrument_info.min_market_order_volume = rsp.min_market_volume();

  if (IsValidInsName(rsp.instrument_id())) {
    market_server.ROLE(InstrumentInfo).BuildInstrumentInfo(rsp.instrument_id(), instrument_info);
  }

  if (rsp.finish_flag()) {
    market_server.ROLE(InstrumentInfo).ShowInstrumentInfo();

    auto &global_sem = GlobalSem::GetInstance();
    global_sem.PostSemBySemName(SemName::kUpdateInstrumentInfo);
  }
}

void TraderEvent::MarketStateRspHandle(utils::ItpMsg &msg) {
  market_trader::message message;
  auto &market_ser = MarketService::GetInstance();
  message.ParseFromString(msg.pb_msg);
  auto result = message.market_state_rsp().result();
  if (result == 0) {
    ERROR_LOG("market state rsp error.");
  }

  market_ser.ROLE(PublishState).ClearPublishFlag();
}

bool TraderEvent::IsValidInsName(const std::string &name) {
  bool ret = true;
  uint32_t prev_type = 0;
  uint32_t count = 0;
  for (auto &item : name) {
    if ('0' <= item && item <= '9') {
      if (prev_type != 1) {
        count++;
      }
      prev_type = 1;
    } else if (('a' <= item && item <= 'z') || ('A' <= item && item <= 'Z')) {
      if (prev_type != 2) {
        count++;
      }
      prev_type = 2;
    } else if (item == '-' || item == '_') {
      continue;
    } else if (item == ' ' || item == '(' || item == ')') {
      ret = false;
      break;
    }
  }
  if (count != 2 && count != 4) {
    ret = false;
  }

  return ret;
}