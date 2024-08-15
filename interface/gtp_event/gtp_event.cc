/*
 * gtpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/gtp_event/gtp_event.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"

#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>

GtpEvent::GtpEvent() {
  RegMsgFun();

  auto &json_cfg = utils::JsonConfig::GetInstance();
  req_instrument_from_ = json_cfg.GetConfig("market", "SubscribeMarketDataFrom").get<std::string>();
}

void GtpEvent::RegMsgFun() {
  msg_func_map_.clear();
  msg_func_map_["OnDepthMarketData"] = [this](utils::ItpMsg &msg) { OnDepthMarketDataHandle(msg); };
  msg_func_map_["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msg_func_map_["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };
  msg_func_map_["OnRspAllInstrumentInfo"] = [this](utils::ItpMsg &msg) { OnRspAllInstrumentInfoHandle(msg); };
}

void GtpEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map_.find(msg.msg_name);
  if (iter != msg_func_map_.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void GtpEvent::OnDepthMarketDataHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto deepdata = reinterpret_cast<GtpMarketDataStruct *>(itp_msg.address());
  auto &market_ser = MarketService::GetInstance();

  if (req_instrument_from_ == "api") {
    if (market_ser.ROLE(MarketTimeState).GetTimeState() == kLoginTime) {
      market_ser.ROLE(LoadData).LoadDepthMarketDataToCsv(deepdata);
    }
  } else {
    if (!block_control_.block ||
        (block_control_.block && block_control_.instruments.find(deepdata->instrument_id) == block_control_.instruments.end())) {
      market_ser.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
    }
  }
}

void GtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<GtpLoginLogoutStruct *>(itp_msg.address());
  if (rsp_info != nullptr) {
    ;
  }

  auto &market_ser = MarketService::GetInstance();
  if (req_instrument_from_ == "local") {
    market_ser.ROLE(SubscribeManager).ReqInstrumentsFromLocal();
  } else if (req_instrument_from_ == "api") {
    market_ser.ROLE(SubscribeManager).ReqInstrumentsFromApi();
  } else if (req_instrument_from_ == "strategy") {
    market_ser.ROLE(SubscribeManager).ReqInstrumrntFromControlPara();
  }
}

void GtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<GtpLoginLogoutStruct *>(itp_msg.address());
  if (rsp_info != nullptr) {
    ;
  }

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(SubscribeManager).UnSubscribeAll();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  if (req_instrument_from_ == "api" && market_ser.ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
    market_ser.ROLE(LoadData).ClassifyContractFiles();
  }

  market_ser.ROLE(InstrumentInfo).EraseAllInstrumentInfo();
}

void GtpEvent::OnRspAllInstrumentInfoHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto gtpqsi = reinterpret_cast<GtpInstrumentInfo *>(itp_msg.address());

  InstrumentInfo::Info instrument_info;
  instrument_info.exch = gtpqsi->exchange_id;
  instrument_info.is_trading = true;
  instrument_info.tradeuint = gtpqsi->tradeuint;
  instrument_info.ticksize = gtpqsi->ticksize;
  instrument_info.max_limit_order_volume = gtpqsi->buy_volume_max;
  instrument_info.min_limit_order_volume = gtpqsi->buy_volume_min;
  instrument_info.max_market_order_volume = gtpqsi->buy_volume_max;
  instrument_info.min_market_order_volume = gtpqsi->buy_volume_min;

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(InstrumentInfo).BuildInstrumentInfo(gtpqsi->instrument_id, instrument_info);

  if (gtpqsi->is_last) {
    market_ser.ROLE(InstrumentInfo).ShowInstrumentInfo();
  }
}

void GtpEvent::SetBlockControl(const std::string &ins, ctpview_market::BlockControl_Command command) {
  if (block_control_.instruments.find(ins) != block_control_.instruments.end() && command == ctpview_market::BlockControl::unblock) {
    block_control_.instruments.erase(ins);
  } else if (block_control_.instruments.find(ins) == block_control_.instruments.end() && command == ctpview_market::BlockControl::block) {
    block_control_.instruments.insert(ins);
  }
  block_control_.block = block_control_.instruments.size() > 0;
}
