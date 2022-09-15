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
  auto iter = msg_func_map.find(msg.msg_name);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void StrategyEvent::TickSubscribeReqHandle(utils::ItpMsg &msg) {
  auto &market_ser = MarketService::GetInstance();
  vector<utils::InstrumtntID> ins_vec;
  ins_vec.clear();

  strategy_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto req_info = message.tick_sub_req();

  for (int i = 0; i < req_info.instrument_info_list_size(); i++) {
    utils::InstrumtntID ins_id;

    ins_id.ins = req_info.instrument_info_list(i).instrument_id();
    ins_id.exch = req_info.instrument_info_list(i).exchange_id();
    ins_id.ticksize = req_info.instrument_info_list(i).ticksize();
    ins_vec.push_back(ins_id);

    PublishControl p_c;
    p_c.prid = req_info.process_random_id();
    p_c.exch = req_info.instrument_info_list(i).exchange_id();
    p_c.ticksize = req_info.instrument_info_list(i).ticksize();
    p_c.indication = strategy_market::TickStartStopIndication_MessageType_reserve;
    p_c.source = req_info.source();
    p_c.heartbeat = 0;

    if (req_info.interval() == "raw") {
      p_c.directforward = true;
    } else {
      p_c.directforward = false;
      p_c.interval = float(std::atof(req_info.interval().c_str()));
    }
    market_ser.ROLE(ControlPara).BuildControlPara(ins_id.ins, p_c);
  }

  if (market_ser.login_state == kLoginState) {
    market_ser.ROLE(SubscribeManager).SubscribeInstrument(ins_vec);
  } else {
    WARNING_LOG("now is logout, wait login to subscribe new instruments");
  }
}

void StrategyEvent::TickStartStopIndicationHandle(utils::ItpMsg &msg) {
  string mapkeyname = "";
  strategy_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto indication = message.tick_start_stop_indication();

  mapkeyname = indication.process_random_id();
  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(ControlPara).SetStartStopIndication(mapkeyname, indication.type());
}

void StrategyEvent::StrategyAliveRspHandle(utils::ItpMsg &msg) { GlobalSem::GetInstance().PostSemBySemName(GlobalSem::kViewDebug); }
