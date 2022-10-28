/*
 * XtpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/xtp_event/xtp_event.h"
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

XtpEvent::XtpEvent() {
  RegMsgFun();
  auto &json_cfg = utils::JsonConfig::GetInstance();
  req_instrument_from_ = json_cfg.GetConfig("market", "SubscribeMarketDataFrom").get<std::string>();
}

void XtpEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["OnDepthMarketData"] = [this](utils::ItpMsg &msg) { OnDepthMarketDataHandle(msg); };
  msg_func_map["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msg_func_map["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };
  msg_func_map["OnQueryAllTickers"] = [this](utils::ItpMsg &msg) { OnQueryAllTickersHandle(msg); };
  msg_func_map["OnRspInstrumentInfo"] = [this](utils::ItpMsg &msg) { OnRspInstrumentInfoHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void XtpEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msg_name);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msg_name.c_str());
  return;
}

void XtpEvent::OnDepthMarketDataHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto deepdata = reinterpret_cast<XTPMD *>(itp_msg.address());
  auto &market_ser = MarketService::GetInstance();

  if (req_instrument_from_ == "api") {
    market_ser.ROLE(LoadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control_ == ctpview_market::BlockControl_Command_unblock) {
      market_ser.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
    }
  }
}

void XtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto xtpri = reinterpret_cast<XTPRI *>(itp_msg.address());
  if (xtpri->error_id != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", xtpri->error_id, xtpri->error_msg);
    exit(-1);
  } else {
    auto &market_ser = MarketService::GetInstance();
    market_ser.ROLE(PublishState).PublishEvent();

    if (req_instrument_from_ == "local") {
      market_ser.ROLE(SubscribeManager).ReqInstrumentsFromLocal();
    } else if (req_instrument_from_ == "api") {
      market_ser.ROLE(SubscribeManager).ReqInstrumentsFromApi();
    } else if (req_instrument_from_ == "strategy") {
      market_ser.ROLE(SubscribeManager).ReqInstrumrntFromControlPara();
    }
  }
}

void XtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto xtpri = reinterpret_cast<XTPRI *>(itp_msg.address());
  if (xtpri->error_id != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", xtpri->error_id, xtpri->error_msg);
    exit(-1);
  } else {
    auto &market_ser = MarketService::GetInstance();
    market_ser.ROLE(SubscribeManager).UnSubscribeAll();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (req_instrument_from_ == "api" && market_ser.ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
      market_ser.ROLE(LoadData).ClassifyContractFiles();
    }

    market_ser.ROLE(InstrumentInfo).EraseAllInstrumentInfo();

    market_ser.ROLE(PublishState).PublishEvent();
  }
}

void XtpEvent::OnQueryAllTickersHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();
  auto xtpqsi = reinterpret_cast<XTPQSI *>(itp_msg.address());

  InstrumentInfo::Info instrument_info;
  if (xtpqsi->exchange_id == XTP_EXCHANGE_SH) {
    instrument_info.exch = "SHSE";
  } else if (xtpqsi->exchange_id == XTP_EXCHANGE_SZ) {
    instrument_info.exch = "SZSE";
  } else {
    ERROR_LOG("not exist exch: %d", xtpqsi->exchange_id);
    instrument_info.exch = "";
  }
  instrument_info.is_trading = true;
  instrument_info.tradeuint = xtpqsi->buy_qty_unit;
  instrument_info.ticksize = xtpqsi->price_tick;
  instrument_info.max_limit_order_volume = xtpqsi->buy_qty_unit;
  instrument_info.min_limit_order_volume = xtpqsi->buy_qty_unit;
  instrument_info.max_market_order_volume = xtpqsi->buy_qty_unit;
  instrument_info.min_market_order_volume = xtpqsi->buy_qty_unit;

  auto &market_server = MarketService::GetInstance();
  market_server.ROLE(InstrumentInfo).BuildInstrumentInfo(xtpqsi->ticker, instrument_info);

  if (itp_msg.is_last()) {
    market_server.ROLE(InstrumentInfo).ShowInstrumentInfo();

    auto &global_sem = GlobalSem::GetInstance();
    global_sem.PostSemBySemName(GlobalSem::kUpdateInstrumentInfo);
  }
}

void XtpEvent::OnRspInstrumentInfoHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto ticker_info = reinterpret_cast<XTPQSI *>(itp_msg.address());
  auto &market_ser = MarketService::GetInstance();

  auto info = market_ser.ROLE(InstrumentInfo).GetInstrumentInfo(ticker_info->ticker);

  strategy_market::message rsp;
  auto *instrument_rsp = rsp.mutable_instrument_rsp();

  instrument_rsp->set_instrument_id(ticker_info->ticker);
  instrument_rsp->set_exchange_id(info->exch);
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

void XtpEvent::SetBlockControl(ctpview_market::BlockControl_Command command) { block_control_ = command; }
