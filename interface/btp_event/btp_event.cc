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
  msg_func_map["OnRspInstrumentInfo"] = [this](utils::ItpMsg &msg) { OnRspInstrumentInfoHandle(msg); };

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
}

void BtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<BtpLoginLogoutStruct *>(itp_msg.address());

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(PublishState).PublishEvent(rsp_info);
}

// btp 具备合约单个信息获取功能，不具备所有合约信息获取功能；将返回合约信息以及构建instrument_info放在一个函数里
void BtpEvent::OnRspInstrumentInfoHandle(utils::ItpMsg &msg) {
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

  auto info = market_ser.ROLE(InstrumentInfo).GetInstrumentInfo(btpqsi->instrument_id);

  strategy_market::message rsp;
  auto *instrument_rsp = rsp.mutable_instrument_rsp();

  instrument_rsp->set_result(strategy_market::Result::success);
  instrument_rsp->set_is_trading(info->is_trading);
  instrument_rsp->set_max_limit_order_volume(info->max_limit_order_volume);
  instrument_rsp->set_max_market_order_volume(info->max_market_order_volume);
  instrument_rsp->set_min_limit_order_volume(info->min_limit_order_volume);
  instrument_rsp->set_min_market_order_volume(info->min_market_order_volume);
  instrument_rsp->set_price_tick(info->ticksize);
  instrument_rsp->set_volume_multiple(info->tradeuint);

  rsp.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "InstrumentRsp." + std::to_string(itp_msg.request_id());
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}
