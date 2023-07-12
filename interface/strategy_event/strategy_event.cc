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
  msg_func_map_["InstrumentReq"] = [this](utils::ItpMsg &msg) { InstrumentReqHandle(msg); };
  msg_func_map_["MarketStateRsp"] = [this](utils::ItpMsg &msg) { MarketStateRspHandle(msg); };
  msg_func_map_["PreProcessStateRsp"] = [this](utils::ItpMsg &msg) { PreProcessStateRspHandle(msg); };

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
  strategy_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto req_info = message.tick_sub_req();

  if (req_info.action() == strategy_market::TickSubscribeReq_Action_sub) {
    vector<utils::InstrumtntID> ins_vec;
    ins_vec.clear();
    utils::InstrumtntID ins_id;

    ins_id.ins = req_info.instrument_info().instrument_id();
    ins_id.exch = req_info.instrument_info().exchange_id();
    ins_vec.push_back(ins_id);

    PublishPara p_a;
    p_a.exch = req_info.instrument_info().exchange_id();
    p_a.source = req_info.source();
    p_a.heartbeat = 0;

    market_ser.ROLE(PublishControl).BuildPublishPara(ins_id.ins, p_a);

    if (market_ser.run_mode == kRealTime) {
      if (market_ser.login_state == kLoginState) {
        market_ser.ROLE(SubscribeManager).SubscribeInstrument(ins_vec);
      } else {
        WARNING_LOG("now is logout, wait login to subscribe new instruments");
      }
    }
  } else if (req_info.action() == strategy_market::TickSubscribeReq_Action_unsub) {
    utils::InstrumtntID ins_id;
    ins_id.exch = req_info.instrument_info().exchange_id();
    ins_id.ins = req_info.instrument_info().instrument_id();
    market_ser.ROLE(PublishControl).ErasePublishPara(ins_id.ins);

    if (market_ser.run_mode == kRealTime) {
      vector<utils::InstrumtntID> ins_vec;
      ins_vec.push_back(ins_id);
      market_ser.ROLE(SubscribeManager).UnSubscribeInstrument(ins_vec);
    }
  }
}

void StrategyEvent::InstrumentReqHandle(utils::ItpMsg &msg) {
  strategy_market::message message;
  message.ParseFromString(msg.pb_msg);
  std::string ins = message.instrument_req().instrument_info().instrument_id();
  std::string exch = message.instrument_req().instrument_info().exchange_id();
  auto &market_ser = MarketService::GetInstance();
  strategy_market::message rsp;
  auto *instrument_rsp = rsp.mutable_instrument_rsp();
  if (market_ser.login_state != kLoginState) {
    instrument_rsp->set_instrument_id(ins);
    instrument_rsp->set_exchange_id(exch);
    instrument_rsp->set_result(strategy_market::Result::failed);
    instrument_rsp->set_failedreason("not login.");
  } else {
    auto info = market_ser.ROLE(InstrumentInfo).GetInstrumentInfo(ins);
    if (info == nullptr) {
      instrument_rsp->set_instrument_id(ins);
      instrument_rsp->set_exchange_id(exch);
      instrument_rsp->set_result(strategy_market::Result::failed);
      instrument_rsp->set_failedreason("not find instrument info.");
    } else {
      instrument_rsp->set_instrument_id(ins);
      instrument_rsp->set_exchange_id(exch);
      instrument_rsp->set_result(strategy_market::Result::success);
      instrument_rsp->set_is_trading(info->is_trading);
      instrument_rsp->set_max_limit_order_volume(info->max_limit_order_volume);
      instrument_rsp->set_max_market_order_volume(info->max_market_order_volume);
      instrument_rsp->set_min_limit_order_volume(info->min_limit_order_volume);
      instrument_rsp->set_min_market_order_volume(info->min_market_order_volume);
      instrument_rsp->set_price_tick(info->ticksize);
      instrument_rsp->set_volume_multiple(info->tradeuint);
    }
  }

  rsp.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "InstrumentRsp";
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);
}

void StrategyEvent::MarketStateRspHandle(utils::ItpMsg &msg) {
  strategy_market::message message;
  auto &market_ser = MarketService::GetInstance();
  message.ParseFromString(msg.pb_msg);
  auto result = message.market_state_rsp().result();
  if (result == 0) {
    ERROR_LOG("market state rsp error.");
  }

  market_ser.ROLE(PublishState).ClearPublishFlag();
}

void StrategyEvent::PreProcessStateRspHandle(utils::ItpMsg &msg) {
  INFO_LOG("post sem name: kStrategyRsp");
  GlobalSem::GetInstance().PostSemBySemName(GlobalSem::kStrategyRsp);
}