/*
 * ctpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/ctp_event/ctp_event.h"
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

CtpEvent::CtpEvent() {
  RegMsgFun();

  auto &jsonCfg = utils::JsonConfig::getInstance();
  req_instrument_from = jsonCfg.get_config("market", "SubscribeMarketDataFrom").get<std::string>();
  INFO_LOG("SubscribeMarketDataFrom: %s", req_instrument_from.c_str());
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
  auto iter = msg_func_map.find(msg.msgName);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void CtpEvent::DeepMarktDataHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto deepdata = (CThostFtdcDepthMarketDataField *)itp_msg.address();
  auto &marketSer = MarketService::getInstance();

  if (req_instrument_from == "api") {
    marketSer.ROLE(LoadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control == ctpview_market::BlockControl_Command_unblock) {
      marketSer.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
    }
  }
}

void CtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto rspInfo = reinterpret_cast<CThostFtdcRspInfoField *>(itp_msg.address());
  TThostFtdcErrorMsgType errormsg;
  utils::Gbk2Utf8(rspInfo->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息
  if (rspInfo->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", rspInfo->ErrorID, errormsg);
    exit(-1);
  } else {
    auto &marketSer = MarketService::getInstance();
    marketSer.ROLE(PublishState).publish_event();

    if (req_instrument_from == "local") {
      marketSer.ROLE(SubscribeManager).reqInstrumentsFromLocal();
    } else if (req_instrument_from == "api") {
      marketSer.ROLE(SubscribeManager).reqInstrumentsFromTrader();
    } else if (req_instrument_from == "strategy") {
      marketSer.ROLE(SubscribeManager).reqInstrumrntFromControlPara();
    }
  }
}

void CtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto rspInfo = reinterpret_cast<CThostFtdcRspInfoField *>(itp_msg.address());
  TThostFtdcErrorMsgType errormsg;
  utils::Gbk2Utf8(rspInfo->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息

  if (rspInfo->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to logout, errorcode=%d errormsg=%s", rspInfo->ErrorID, errormsg);
    exit(-1);
  } else {
    auto &marketSer = MarketService::getInstance();
    marketSer.ROLE(SubscribeManager).unSubscribeAll();

    std::this_thread::sleep_for(1000ms);

    if (req_instrument_from == "trader" && marketSer.ROLE(MarketTimeState).get_time_state() == kLogoutTime) {
      marketSer.ROLE(LoadData).ClassifyContractFiles();
    }

    marketSer.ROLE(LoadData).ClearInsExchPair();

    marketSer.ROLE(PublishState).publish_event();
  }
}

void CtpEvent::set_block_control(ctpview_market::BlockControl_Command command) { block_control = command; }
