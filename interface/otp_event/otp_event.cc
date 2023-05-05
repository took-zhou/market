/*
 * otpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/otp_event/otp_event.h"
#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/extern/otp/inc/mds_api/mds_async_api.h"
#include "common/self/file_util.h"
#include "common/self/profiler.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

OtpEvent::OtpEvent() {
  RegMsgFun();

  auto &json_cfg = utils::JsonConfig::GetInstance();
  req_instrument_from_ = json_cfg.GetConfig("market", "SubscribeMarketDataFrom").get<std::string>();
}

void OtpEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["OnDepthMarketData"] = [this](utils::ItpMsg &msg) { OnDepthMarketDataHandle(msg); };
  msg_func_map["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msg_func_map["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };
  msg_func_map["OnRspStockStaticInfo"] = [this](utils::ItpMsg &msg) { OnRspStockStaticInfoHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void OtpEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msg_name);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void OtpEvent::OnDepthMarketDataHandle(utils::ItpMsg &msg) {
  PZone("OnDepthMarketDataHandle");
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto deepdata = (MdsMktDataSnapshotT *)itp_msg.address();
  auto &market_ser = MarketService::GetInstance();

  if (req_instrument_from_ == "api") {
    market_ser.ROLE(LoadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control_ == ctpview_market::BlockControl_Command_unblock) {
      market_ser.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
    }
  }
}

void OtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  auto &market_ser = MarketService::GetInstance();
  if (req_instrument_from_ == "local") {
    market_ser.ROLE(SubscribeManager).ReqInstrumentsFromLocal();
  } else if (req_instrument_from_ == "api") {
    market_ser.ROLE(SubscribeManager).ReqInstrumentsFromApi();
  } else if (req_instrument_from_ == "strategy") {
    market_ser.ROLE(SubscribeManager).ReqInstrumrntFromControlPara();
  }
}

void OtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(SubscribeManager).UnSubscribeAll();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (req_instrument_from_ == "api" && market_ser.ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
    market_ser.ROLE(LoadData).ClassifyContractFiles();
  }
  market_ser.ROLE(InstrumentInfo).EraseAllInstrumentInfo();
}

void OtpEvent::OnRspStockStaticInfoHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();
  auto static_info = reinterpret_cast<MdsStockStaticInfoT *>(itp_msg.address());

  InstrumentInfo::Info instrument_info;
  if (static_info->exchId == MDS_EXCH_SSE) {
    instrument_info.exch = "SHSE";
  } else if (static_info->exchId == MDS_EXCH_SZSE) {
    instrument_info.exch = "SZSE";
  } else {
    ERROR_LOG("not exist exch: %d", static_info->exchId);
    instrument_info.exch = "";
  }
  instrument_info.is_trading = true;
  instrument_info.tradeuint = static_info->lmtBuyQtyUnit;
  instrument_info.ticksize = static_info->priceTick * 0.0001;
  instrument_info.max_limit_order_volume = static_info->lmtBuyMaxQty;
  instrument_info.min_limit_order_volume = static_info->lmtBuyMinQty;
  instrument_info.max_market_order_volume = static_info->mktBuyMaxQty;
  instrument_info.min_market_order_volume = static_info->mktBuyMinQty;

  auto &market_server = MarketService::GetInstance();
  market_server.ROLE(InstrumentInfo).BuildInstrumentInfo(static_info->securityId, instrument_info);

  if (itp_msg.is_last()) {
    market_server.ROLE(InstrumentInfo).ShowInstrumentInfo();

    auto &global_sem = GlobalSem::GetInstance();
    global_sem.PostSemBySemName(GlobalSem::kUpdateInstrumentInfo);
  }
}

void OtpEvent::SetBlockControl(ctpview_market::BlockControl_Command command) { block_control_ = command; }
