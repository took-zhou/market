/*
 * traderEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/traderEvent/traderEvent.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/utils.h"
#include "market/domain/marketService.h"

TraderEvent::TraderEvent() { regMsgFun(); }

void TraderEvent::handle(utils::ItpMsg &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void TraderEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["QryInstrumentRsp"] = [this](utils::ItpMsg &msg) { QryInstrumentRspHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
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
    marketServer.ROLE(loadData).insertInsExchPair(instrumtntID.ins, instrumtntID.exch);
    ins_vec.push_back(instrumtntID);

    instrumentCount++;
  }

  if (rsp.finish_flag() == true) {
    marketServer.ROLE(subscribeManager).subscribeInstrument(ins_vec);
    marketServer.ROLE(loadData).showInsExchPair();
    INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
    instrumentCount = 0;
    ins_vec.clear();
  } else if (ins_vec.size() >= 500) {
    INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
    marketServer.ROLE(subscribeManager).subscribeInstrument(ins_vec);
    ins_vec.clear();
  }
}
