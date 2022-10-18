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
  msg_func_map_.clear();
  msg_func_map_["TickSubscribeReq"] = [this](utils::ItpMsg &msg) { TickSubscribeReqHandle(msg); };
  msg_func_map_["TickStartStopIndication"] = [this](utils::ItpMsg &msg) { TickStartStopIndicationHandle(msg); };
  msg_func_map_["ActiveSafetyRsp"] = [this](utils::ItpMsg &msg) { StrategyAliveRspHandle(msg); };
  msg_func_map_["InstrumentReq"] = [this](utils::ItpMsg &msg) { InstrumentReqHandle(msg); };

  for (auto &iter : msg_func_map_) {
    INFO_LOG("msg_func_map_[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void StrategyEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map_.find(msg.msg_name);
  if (iter != msg_func_map_.end()) {
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

  utils::InstrumtntID ins_id;

  ins_id.ins = req_info.instrument_info().instrument_id();
  ins_id.exch = req_info.instrument_info().exchange_id();
  ins_vec.push_back(ins_id);

  PublishControl p_c;
  p_c.prid = req_info.process_random_id();
  p_c.exch = req_info.instrument_info().exchange_id();
  p_c.indication = strategy_market::TickStartStopIndication_MessageType_reserve;
  p_c.source = req_info.source();
  p_c.heartbeat = 0;
  p_c.begin = req_info.begin_time();
  p_c.end = req_info.end_time();
  p_c.speed = req_info.speed();

  if (req_info.interval() == "raw") {
    p_c.directforward = true;
  } else {
    p_c.directforward = false;
    p_c.interval = float(std::atof(req_info.interval().c_str()));
  }
  market_ser.ROLE(ControlPara).BuildControlPara(ins_id.ins, p_c);

  if (market_ser.login_state == kLoginState) {
    market_ser.ROLE(SubscribeManager).SubscribeInstrument(ins_vec, stoi(req_info.process_random_id()));
  } else {
    WARNING_LOG("now is logout, wait login to subscribe new instruments");
  }
}

void StrategyEvent::TickStartStopIndicationHandle(utils::ItpMsg &msg) {
  strategy_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto indication = message.tick_start_stop_indication();

  string prid = indication.process_random_id();
  string ins = indication.instrument_info().instrument_id();
  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(ControlPara).SetStartStopIndication(prid, indication.type());
  if (indication.type() == strategy_market::TickStartStopIndication_MessageType_finish) {
    auto instrument_list = market_ser.ROLE(ControlPara).GetInstrumentList(prid);
    for (auto &item : instrument_list) {
      if (market_ser.ROLE(ControlPara).GetInstrumentSubscribedCount(item) == 1) {
        vector<utils::InstrumtntID> ins_vec;
        ins_vec.push_back(item);
        market_ser.ROLE(SubscribeManager).UnSubscribeInstrument(ins_vec, stoi(prid));
      }
    }

    market_ser.ROLE(ControlPara).EraseControlPara(prid, ins);
  }
}

void StrategyEvent::InstrumentReqHandle(utils::ItpMsg &msg) {
  strategy_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto prid = message.instrument_req().process_random_id();
  auto &market_ser = MarketService::GetInstance();
  if (market_ser.login_state != kLoginState) {
    ERROR_LOG("itp not login!");
    return;
  }

  utils::InstrumtntID ins_exch;
  ins_exch.ins = message.instrument_req().instrument_info().instrument_id();
  ins_exch.exch = message.instrument_req().instrument_info().exchange_id();

  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).ReqInstrumentInfo(ins_exch, stoi(prid));
}

void StrategyEvent::StrategyAliveRspHandle(utils::ItpMsg &msg) { GlobalSem::GetInstance().PostSemBySemName(GlobalSem::kViewDebug); }
