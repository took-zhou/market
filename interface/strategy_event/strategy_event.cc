/*
 * strategyEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "common/extern/log/log.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

#include "common/self/semaphore.h"
#include "market/domain/market_service.h"
#include "market/interface/strategy_event/strategy_event.h"

StrategyEvent::StrategyEvent() { RegMsgFun(); }

void StrategyEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["TickSubscribeReq"] = [this](utils::ItpMsg &msg) { TickSubscribeReqHandle(msg); };
  msg_func_map["TickStartStopIndication"] = [this](utils::ItpMsg &msg) { TickStartStopIndicationHandle(msg); };
  msg_func_map["ActiveSafetyRsp"] = [this](utils::ItpMsg &msg) { StrategyAliveRspHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void StrategyEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msgName);
  if (iter != msg_func_map.end()) {
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

    PublishControl pc;
    pc.prid = reqInfo.process_random_id();
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
    marketSer.ROLE(ControlPara).BuildControlPara(insId.ins, pc);
  }

  if (marketSer.login_state == kLoginState) {
    marketSer.ROLE(SubscribeManager).subscribeInstrument(insVec);
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
  marketSer.ROLE(ControlPara).set_start_stop_indication(mapkeyname, indication.type());
}

void StrategyEvent::StrategyAliveRspHandle(utils::ItpMsg &msg) { GlobalSem::getInstance().PostSemBySemName(GlobalSem::kViewDebug); }
