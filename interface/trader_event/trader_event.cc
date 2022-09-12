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
  auto iter = msg_func_map.find(msg.msgName);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
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
  static int instrumentCount;
  static vector<utils::InstrumtntID> ins_vec;
  auto &marketServer = MarketService::getInstance();

  market_trader::message rspMsg;
  rspMsg.ParseFromString(msg.pbMsg);

  auto &rsp = rspMsg.qry_instrument_rsp();

  utils::InstrumtntID instrumtntID;
  instrumtntID.exch = rsp.exchange_id();
  instrumtntID.ins = rsp.instrument_id();

  if (instrumtntID.ins.find(" ") == instrumtntID.ins.npos) {
    marketServer.ROLE(LoadData).InsertInsExchPair(instrumtntID.ins, instrumtntID.exch);
    ins_vec.push_back(instrumtntID);

    instrumentCount++;
  }

  if (rsp.finish_flag() == true) {
    marketServer.ROLE(SubscribeManager).subscribeInstrument(ins_vec);
    marketServer.ROLE(LoadData).ShowInsExchPair();
    INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
    instrumentCount = 0;
    ins_vec.clear();
  } else if (ins_vec.size() >= 500) {
    INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
    marketServer.ROLE(SubscribeManager).subscribeInstrument(ins_vec);
    ins_vec.clear();
  }
}
