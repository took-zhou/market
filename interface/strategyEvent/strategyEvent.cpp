/*
 * strategyEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "common/extern/log/log.h"
#include "common/self/protobuf/market-strategy.pb.h"
#include "common/self/utils.h"

#include "common/self/semaphorePart.h"
#include "market/domain/components/ctpMarketApi/ctpMarketApi.h"
#include "market/domain/marketService.h"
#include "market/infra/define.h"
#include "market/interface/strategyEvent/strategyEvent.h"
extern GlobalSem globalSem;

bool StrategyEvent::init() {
  regMsgFun();

  return true;
}

void StrategyEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["TickSubscribeReq"] = [this](MsgStruct &msg) { TickSubscribeReqHandle(msg); };
  msgFuncMap["TickStartStopIndication"] = [this](MsgStruct &msg) { TickStartStopIndicationHandle(msg); };
  msgFuncMap["ActiveSafetyRsp"] = [this](MsgStruct &msg) { StrategyAliveRspHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void StrategyEvent::handle(MsgStruct &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void StrategyEvent::TickSubscribeReqHandle(MsgStruct &msg) {
  vector<string> keywordVec;
  vector<utils::InstrumtntID> insVec;
  string mapkeyname = "";
  insVec.clear();
  keywordVec.clear();

  market_strategy::message _reqInfo;
  _reqInfo.ParseFromString(msg.pbMsg);
  auto reqInfo = _reqInfo.tick_sub_req();

  for (int i = 0; i < reqInfo.instrument_info_list_size(); i++) {
    utils::InstrumtntID insId;
    insId.ins = reqInfo.instrument_info_list(i).instrument_id();
    insId.exch = reqInfo.instrument_info_list(i).exchange_id();
    insId.ticksize = utils::stringToFloat(reqInfo.instrument_info_list(i).ticksize());
    insVec.push_back(insId);
  }

  mapkeyname = reqInfo.process_random_id();

  auto &marketSer = MarketService::getInstance();
  marketSer.ROLE(controlPara).buildInstrumentList(mapkeyname, insVec);

  if (reqInfo.interval() == "raw") {
    marketSer.ROLE(controlPara).setDirectForwardingFlag(mapkeyname, true);
    marketSer.ROLE(controlPara).setSource(mapkeyname, reqInfo.source());
  } else {
    marketSer.ROLE(controlPara).setInterval(mapkeyname, float(std::atof(reqInfo.interval().c_str())));
    marketSer.ROLE(controlPara).setDirectForwardingFlag(mapkeyname, false);
  }

  if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE) {
    marketSer.ROLE(Market).ROLE(CtpMarketApi).marketApi->SubscribeMarketData(insVec);
  } else {
    WARNING_LOG("now is logout, wait login to subscribe new instruments");
  }
}

void StrategyEvent::TickStartStopIndicationHandle(MsgStruct &msg) {
  string mapkeyname = "";
  market_strategy::message _indication;
  _indication.ParseFromString(msg.pbMsg);
  auto indication = _indication.tick_start_stop_indication();

  mapkeyname = indication.process_random_id();
  auto &marketSer = MarketService::getInstance();
  marketSer.ROLE(controlPara).setStartStopIndication(mapkeyname, indication.type());
}

void StrategyEvent::StrategyAliveRspHandle(MsgStruct &msg) {
  std::string semName = "req_alive";
  globalSem.postSemBySemName(semName);
  INFO_LOG("post sem of [%s]", semName.c_str());
}
