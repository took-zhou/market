/*
 * btpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/btp_event/btp_event.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>

BtpEvent::BtpEvent() { RegMsgFun(); }

void BtpEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["OnDepthMarketData"] = [this](utils::ItpMsg &msg) { OnDepthMarketDataHandle(msg); };
  msg_func_map["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msg_func_map["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };
  msg_func_map["OnRspAllInstrumentInfo"] = [this](utils::ItpMsg &msg) { OnRspAllInstrumentInfoHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void BtpEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msg_name);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void BtpEvent::OnDepthMarketDataHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto deepdata = (BtpMarketDataStruct *)itp_msg.address();
  auto &market_ser = MarketService::GetInstance();

  market_ser.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
}

void BtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<BtpLoginLogoutStruct *>(itp_msg.address());

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(PublishState).PublishEvent(rsp_info);
  PreProcessStateReq(rsp_info->prid);
}

void BtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<BtpLoginLogoutStruct *>(itp_msg.address());

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(PublishState).PublishEvent(rsp_info);
  PreProcessStateReq(rsp_info->prid);
}

void BtpEvent::OnRspAllInstrumentInfoHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto btpqsi = reinterpret_cast<BtpInstrumentInfo *>(itp_msg.address());

  InstrumentInfo::Info instrument_info;
  instrument_info.exch = btpqsi->exchange_id;
  instrument_info.is_trading = true;
  instrument_info.tradeuint = btpqsi->tradeuint;
  instrument_info.ticksize = btpqsi->ticksize;
  instrument_info.max_limit_order_volume = btpqsi->buy_volume_max;
  instrument_info.min_limit_order_volume = btpqsi->buy_volume_min;
  instrument_info.max_market_order_volume = btpqsi->buy_volume_max;
  instrument_info.min_market_order_volume = btpqsi->buy_volume_min;

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(InstrumentInfo).BuildInstrumentInfo(btpqsi->instrument_id, instrument_info);

  if (btpqsi->is_last == true) {
    market_ser.ROLE(InstrumentInfo).ShowInstrumentInfo();
  }
}

void BtpEvent::PreProcessStateReq(int32_t prid) {
  auto &market_ser = MarketService::GetInstance();
  if (prid == 0) {
    auto key_name_kist = market_ser.ROLE(ControlPara).GetPridList();
    for (auto &keyname : key_name_kist) {
      strategy_market::message tick;
      auto process_state = tick.mutable_pre_process_state_req();
      process_state->set_state_req(1);

      utils::ItpMsg msg;
      tick.SerializeToString(&msg.pb_msg);
      msg.session_name = "strategy_market";
      msg.msg_name = "PreProcessStateReq." + keyname;
      auto &recer_sender = RecerSender::GetInstance();
      recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);

      auto &global_sem = GlobalSem::GetInstance();
      global_sem.WaitSemBySemName(GlobalSem::kStrategyRsp, 300);
    }
  } else {
    strategy_market::message tick;
    auto process_state = tick.mutable_pre_process_state_req();
    process_state->set_state_req(1);

    utils::ItpMsg msg;
    tick.SerializeToString(&msg.pb_msg);
    msg.session_name = "strategy_market";
    msg.msg_name = "PreProcessStateReq." + to_string(prid);
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);

    auto &global_sem = GlobalSem::GetInstance();
    global_sem.WaitSemBySemName(GlobalSem::kStrategyRsp, 300);
  }
}