/*
 * ftpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/ftp_event/ftp_event.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/profiler.h"
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

FtpEvent::FtpEvent() {
  RegMsgFun();

  auto &json_cfg = utils::JsonConfig::GetInstance();
  req_instrument_from_ = json_cfg.GetConfig("market", "SubscribeMarketDataFrom").get<std::string>();
}

void FtpEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map_.clear();
  msg_func_map_["OnDepthMarketData"] = [this](utils::ItpMsg &msg) { OnDepthMarketDataHandle(msg); };
  msg_func_map_["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msg_func_map_["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };
  msg_func_map_["OnRspAllInstrumentInfo"] = [this](utils::ItpMsg &msg) { OnRspAllInstrumentInfoHandle(msg); };

  for (auto &iter : msg_func_map_) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void FtpEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map_.find(msg.msg_name);
  if (iter != msg_func_map_.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void FtpEvent::OnDepthMarketDataHandle(utils::ItpMsg &msg) {
  PZone("DeepMarktDataHandle");
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto deepdata = (FtpMarketDataStruct *)itp_msg.address();
  auto &market_ser = MarketService::GetInstance();

  market_ser.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
}

void FtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<FtpLoginLogoutStruct *>(itp_msg.address());

  auto &market_ser = MarketService::GetInstance();
  if (market_ser.run_mode == kFastBack) {
    market_ser.ROLE(PublishState).PublishEvent(rsp_info);
  } else {
    if (req_instrument_from_ == "local") {
      market_ser.ROLE(SubscribeManager).ReqInstrumentsFromLocal();
    } else if (req_instrument_from_ == "api") {
      market_ser.ROLE(SubscribeManager).ReqInstrumentsFromApi();
    } else if (req_instrument_from_ == "strategy") {
      market_ser.ROLE(SubscribeManager).ReqInstrumrntFromControlPara();
    }
  }
}

void FtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<FtpLoginLogoutStruct *>(itp_msg.address());

  auto &market_ser = MarketService::GetInstance();

  if (market_ser.run_mode == kFastBack) {
    market_ser.ROLE(PublishState).PublishEvent(rsp_info);
  } else {
    market_ser.ROLE(SubscribeManager).UnSubscribeAll();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (req_instrument_from_ == "api" && market_ser.ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
      market_ser.ROLE(LoadData).ClassifyContractFiles();
    }

    market_ser.ROLE(InstrumentInfo).EraseAllInstrumentInfo();
  }
}

void FtpEvent::OnRspAllInstrumentInfoHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto ins_info = reinterpret_cast<FtpInstrumentInfo *>(itp_msg.address());

  InstrumentInfo::Info instrument_info;
  instrument_info.exch = ins_info->exchange_id;
  instrument_info.is_trading = true;
  instrument_info.tradeuint = ins_info->tradeuint;
  instrument_info.ticksize = ins_info->ticksize;
  instrument_info.max_limit_order_volume = ins_info->buy_volume_max;
  instrument_info.min_limit_order_volume = ins_info->buy_volume_min;
  instrument_info.max_market_order_volume = ins_info->buy_volume_max;
  instrument_info.min_market_order_volume = ins_info->buy_volume_min;

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(InstrumentInfo).BuildInstrumentInfo(ins_info->instrument_id, instrument_info);

  if (ins_info->is_last == true) {
    market_ser.ROLE(InstrumentInfo).ShowInstrumentInfo();
  }
}
