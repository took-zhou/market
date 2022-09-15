/*
 * traderEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/trader_event/trader_event.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"

TraderEvent::TraderEvent() { RegMsgFun(); }

void TraderEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msg_name);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msg_name.c_str());
  return;
}

void TraderEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["QryInstrumentRsp"] = [this](utils::ItpMsg &msg) { QryInstrumentRspHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
  return;
}

void TraderEvent::QryInstrumentRspHandle(utils::ItpMsg &msg) {
  static int instrument_count;
  static vector<utils::InstrumtntID> ins_vec;
  auto &market_server = MarketService::GetInstance();

  market_trader::message message;
  message.ParseFromString(msg.pb_msg);

  auto &rsp = message.qry_instrument_rsp();

  utils::InstrumtntID instrumtnt_id;
  instrumtnt_id.exch = rsp.exchange_id();
  instrumtnt_id.ins = rsp.instrument_id();

  if (instrumtnt_id.ins.find(" ") == instrumtnt_id.ins.npos) {
    market_server.ROLE(LoadData).InsertInsExchPair(instrumtnt_id.ins, instrumtnt_id.exch);
    ins_vec.push_back(instrumtnt_id);

    instrument_count++;
  }

  if (rsp.finish_flag() == true) {
    market_server.ROLE(SubscribeManager).SubscribeInstrument(ins_vec);
    market_server.ROLE(LoadData).ShowInsExchPair();
    INFO_LOG("The number of trading contracts is: %d.", instrument_count);
    instrument_count = 0;
    ins_vec.clear();
  } else if (ins_vec.size() >= 500) {
    INFO_LOG("The number of trading contracts is: %d.", instrument_count);
    market_server.ROLE(SubscribeManager).SubscribeInstrument(ins_vec);
    ins_vec.clear();
  }
}
