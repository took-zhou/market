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
      INFO_LOG("reqInstrumentsFromMarket");
      market_ser.ROLE(SubscribeManager).ReqInstrumentsFromMarket();
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

    std::this_thread::sleep_for(1000ms);

    if (req_instrument_from_ == "trader" && market_ser.ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
      market_ser.ROLE(LoadData).ClassifyContractFiles();
    }

    market_ser.ROLE(LoadData).ClearInsExchPair();

    market_ser.ROLE(PublishState).PublishEvent();
  }
}

void XtpEvent::OnQueryAllTickersHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();
  auto xtpqsi = reinterpret_cast<XTPQSI *>(itp_msg.address());
  static int instrument_count;
  static vector<utils::InstrumtntID> ins_vec;
  auto &market_server = MarketService::GetInstance();

  if (!itp_msg.is_last()) {
    utils::InstrumtntID instrumtnt_id;
    if (xtpqsi->exchange_id == XTP_EXCHANGE_SH) {
      instrumtnt_id.exch = "SHSE";
      instrumtnt_id.ins = xtpqsi->ticker;
    } else if (xtpqsi->exchange_id == XTP_EXCHANGE_SZ) {
      instrumtnt_id.exch = "SZSE";
      instrumtnt_id.ins = xtpqsi->ticker;
    } else {
      return;
    }

    if (instrumtnt_id.ins.find(" ") == instrumtnt_id.ins.npos) {
      auto &market_ser = MarketService::GetInstance();
      market_ser.ROLE(LoadData).InsertInsExchPair(instrumtnt_id.ins, instrumtnt_id.exch);
      ins_vec.push_back(instrumtnt_id);

      instrument_count++;
    }

    if (ins_vec.size() >= 500) {
      INFO_LOG("The number of trading contracts is: %d.", instrument_count);
      market_server.ROLE(SubscribeManager).SubscribeInstrument(ins_vec);
      ins_vec.clear();
    }
  } else {
    if (instrument_count > 0) {
      market_server.ROLE(SubscribeManager).SubscribeInstrument(ins_vec);
      INFO_LOG("The number of trading contracts is: %d.", instrument_count);
      instrument_count = 0;
      ins_vec.clear();
    }
  }
}

void XtpEvent::SetBlockControl(ctpview_market::BlockControl_Command command) { block_control_ = command; }
