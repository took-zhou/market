/*
 * ctpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/ctp_event/ctp_event.h"
#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/profiler.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

CtpEvent::CtpEvent() {
  RegMsgFun();

  auto &json_cfg = utils::JsonConfig::GetInstance();
  req_instrument_from_ = json_cfg.GetConfig("market", "SubscribeMarketDataFrom").get<std::string>();
}

void CtpEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["OnRtnDepthMarketData"] = [this](utils::ItpMsg &msg) { DeepMarktDataHandle(msg); };
  msg_func_map["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msg_func_map["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void CtpEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msg_name);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void CtpEvent::DeepMarktDataHandle(utils::ItpMsg &msg) {
  PZone("DeepMarktDataHandle");
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto deepdata = (CThostFtdcDepthMarketDataField *)itp_msg.address();
  auto &market_ser = MarketService::GetInstance();

  if (req_instrument_from_ == "api") {
    market_ser.ROLE(LoadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control_ == ctpview_market::BlockControl_Command_unblock) {
      market_ser.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
    }
  }
}

void CtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<CThostFtdcRspInfoField *>(itp_msg.address());
  TThostFtdcErrorMsgType errormsg;
  utils::Gbk2Utf8(rsp_info->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息
  if (rsp_info->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", rsp_info->ErrorID, errormsg);
    exit(-1);
  } else {
    // 同步全局合约信息
    UpdateInstrumentInfoFromTrader();

    auto &market_ser = MarketService::GetInstance();
    if (req_instrument_from_ == "local") {
      market_ser.ROLE(SubscribeManager).ReqInstrumentsFromLocal();
    } else if (req_instrument_from_ == "api") {
      market_ser.ROLE(SubscribeManager).ReqInstrumentsFromApi();
    } else if (req_instrument_from_ == "strategy") {
      market_ser.ROLE(SubscribeManager).ReqInstrumrntFromControlPara();
    }
  }
}

void CtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message message;
  message.ParseFromString(msg.pb_msg);
  auto &itp_msg = message.itp_msg();

  auto rsp_info = reinterpret_cast<CThostFtdcRspInfoField *>(itp_msg.address());
  TThostFtdcErrorMsgType errormsg;
  utils::Gbk2Utf8(rsp_info->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息

  if (rsp_info->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to logout, errorcode=%d errormsg=%s", rsp_info->ErrorID, errormsg);
    exit(-1);
  } else {
    auto &market_ser = MarketService::GetInstance();
    market_ser.ROLE(SubscribeManager).UnSubscribeAll();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (req_instrument_from_ == "api" && market_ser.ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
      market_ser.ROLE(LoadData).ClassifyContractFiles();
    }

    market_ser.ROLE(InstrumentInfo).EraseAllInstrumentInfo();
  }
}

void CtpEvent::UpdateInstrumentInfoFromTrader() {
  market_trader::message req_msg;
  auto req_instrument = req_msg.mutable_qry_instrument_req();
  req_instrument->set_identity("all");

  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "market_trader";
  msg.msg_name = "QryInstrumentReq";

  auto &recer_sender = RecerSender::GetInstance();
  auto &global_sem = GlobalSem::GetInstance();
  for (uint8_t wait_count = 0; wait_count < 3; wait_count++) {
    recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);
    INFO_LOG("update instrument info from trader send ok, waiting trader rsp.");
    if (!global_sem.WaitSemBySemName(GlobalSem::kUpdateInstrumentInfo, 60)) {
      break;
    }
  }
}

void CtpEvent::SetBlockControl(ctpview_market::BlockControl_Command command) { block_control_ = command; }
