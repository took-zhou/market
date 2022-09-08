/*
 * strategyEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "common/extern/log/log.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

#include "common/self/semaphorePart.h"
#include "market/domain/marketService.h"
#include "market/interface/strategyEvent/strategyEvent.h"

StrategyEvent::StrategyEvent() { regMsgFun(); }

void StrategyEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["TickSubscribeReq"] = [this](utils::ItpMsg &msg) { TickSubscribeReqHandle(msg); };
  msgFuncMap["TickStartStopIndication"] = [this](utils::ItpMsg &msg) { TickStartStopIndicationHandle(msg); };
  msgFuncMap["ActiveSafetyRsp"] = [this](utils::ItpMsg &msg) { StrategyAliveRspHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void StrategyEvent::handle(utils::ItpMsg &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void StrategyEvent::TickSubscribeReqHandle(utils::ItpMsg &msg) {
  auto &marketSer = MarketService::getInstance();
  vector<utils::InstrumtntID> insVec;
  insVec.clear();

  strategy_market::message _reqInfo;
  _reqInfo.ParseFromString(msg.pbMsg);
  auto reqInfo = _reqInfo.tick_sub_req();

  for (int i = 0; i < reqInfo.instrument_info_list_size(); i++) {
    utils::InstrumtntID insId;

    insId.ins = reqInfo.instrument_info_list(i).instrument_id();
    insId.exch = reqInfo.instrument_info_list(i).exchange_id();
    insId.ticksize = reqInfo.instrument_info_list(i).ticksize();
    insVec.push_back(insId);

    publishControl pc;
    pc.identify = reqInfo.process_random_id();
    pc.exch = reqInfo.instrument_info_list(i).exchange_id();
    pc.ticksize = reqInfo.instrument_info_list(i).ticksize();
    pc.indication = strategy_market::TickStartStopIndication_MessageType_reserve;
    pc.source = reqInfo.source();
    pc.heartbeat = 0;

    if (reqInfo.interval() == "raw") {
      pc.directforward = true;
    } else {
      pc.directforward = false;
      pc.interval = float(std::atof(reqInfo.interval().c_str()));
    }
    marketSer.ROLE(controlPara).buildControlPara(insId.ins, pc);
  }

  if (marketSer.login_state == LOGIN_STATE) {
    marketSer.ROLE(subscribeManager).subscribeInstrument(insVec);
  } else {
    WARNING_LOG("now is logout, wait login to subscribe new instruments");
  }
}

void StrategyEvent::TickStartStopIndicationHandle(utils::ItpMsg &msg) {
  string mapkeyname = "";
  strategy_market::message _indication;
  _indication.ParseFromString(msg.pbMsg);
  auto indication = _indication.tick_start_stop_indication();

  mapkeyname = indication.process_random_id();
  auto &marketSer = MarketService::getInstance();
  marketSer.ROLE(controlPara).setStartStopIndication(mapkeyname, indication.type());
}

void StrategyEvent::StrategyAliveRspHandle(utils::ItpMsg &msg) { GlobalSem::getInstance().postSemBySemName(GlobalSem::viewDebug); }
