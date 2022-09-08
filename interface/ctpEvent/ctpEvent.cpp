/*
 * ctpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/ctpEvent/ctpEvent.h"
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/domain/marketService.h"
#include "market/infra/recerSender.h"

#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>

CtpEvent::CtpEvent() {
  regMsgFun();

  auto &jsonCfg = utils::JsonConfig::getInstance();
  reqInstrumentFrom = jsonCfg.getConfig("market", "SubscribeMarketDataFrom").get<std::string>();
  INFO_LOG("SubscribeMarketDataFrom: %s", reqInstrumentFrom.c_str());
}

void CtpEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["OnRtnDepthMarketData"] = [this](utils::ItpMsg &msg) { DeepMarktDataHandle(msg); };
  msgFuncMap["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msgFuncMap["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void CtpEvent::handle(utils::ItpMsg &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
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

  if (reqInstrumentFrom == "api") {
    marketSer.ROLE(loadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control == ctpview_market::BlockControl_Command_unblock) {
      marketSer.ROLE(publishData).directForwardDataToStrategy(deepdata);
    }
  }
}

void CtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto rspInfo = reinterpret_cast<CThostFtdcRspInfoField *>(itp_msg.address());
  TThostFtdcErrorMsgType errormsg;
  utils::gbk2utf8(rspInfo->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息
  if (rspInfo->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", rspInfo->ErrorID, errormsg);
    exit(-1);
  } else {
    auto &marketSer = MarketService::getInstance();
    marketSer.ROLE(publishState).publish_event();

    if (reqInstrumentFrom == "local") {
      marketSer.ROLE(subscribeManager).reqInstrumentsFromLocal();
    } else if (reqInstrumentFrom == "api") {
      marketSer.ROLE(subscribeManager).reqInstrumentsFromTrader();
    } else if (reqInstrumentFrom == "strategy") {
      marketSer.ROLE(subscribeManager).reqInstrumrntFromControlPara();
    }
  }
}

void CtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto rspInfo = reinterpret_cast<CThostFtdcRspInfoField *>(itp_msg.address());
  TThostFtdcErrorMsgType errormsg;
  utils::gbk2utf8(rspInfo->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息

  if (rspInfo->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to logout, errorcode=%d errormsg=%s", rspInfo->ErrorID, errormsg);
    exit(-1);
  } else {
    auto &marketSer = MarketService::getInstance();
    marketSer.ROLE(subscribeManager).unSubscribeAll();

    std::this_thread::sleep_for(1000ms);

    if (reqInstrumentFrom == "trader" && (marketSer.ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_logout ||
                                          marketSer.ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_logout)) {
      marketSer.ROLE(loadData).ClassifyContractFiles();
    }

    marketSer.ROLE(loadData).clearInsExchPair();

    marketSer.ROLE(publishState).publish_event();
  }
}

void CtpEvent::set_block_control(ctpview_market::BlockControl_Command command) { block_control = command; }
