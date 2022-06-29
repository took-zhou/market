/*
 * ctpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/ctpEvent/ctpEvent.h"
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/domain/marketService.h"
#include "market/infra/define.h"

#include <unistd.h>
#include <sstream>
#include <string>

extern GlobalSem globalSem;
constexpr U32 WAITTIME_FOR_CONTRACTS_UNSCRIBEED = 1;

bool CtpEvent::init() {
  regMsgFun();

  auto &jsonCfg = utils::JsonConfig::getInstance();
  reqInstrumentFrom = jsonCfg.getConfig("market", "SubscribeMarketDataFrom").get<std::string>();
  return true;
}

void CtpEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["OnRtnDepthMarketData"] = [this](MsgStruct &msg) { DeepMarktDataHandle(msg); };
  msgFuncMap["LoginInfo"] = [this](MsgStruct &msg) { LoginInfoHandle(msg); };
  msgFuncMap["LogoutInfo"] = [this](MsgStruct &msg) { LogoutInfoHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void CtpEvent::handle(MsgStruct &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void CtpEvent::DeepMarktDataHandle(MsgStruct &msg) {
  auto deepdata = (CThostFtdcDepthMarketDataField *)msg.ctpMsg;
  auto &marketSer = MarketService::getInstance();

  if (reqInstrumentFrom == "trader") {
    marketSer.ROLE(loadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control == ctpview_market::BlockControl_Command_unblock) {
      marketSer.ROLE(publishData).directForwardDataToStrategy(deepdata);
    }
  }
}

void CtpEvent::LoginInfoHandle(MsgStruct &msg) {
  auto ctpMsg = (CThostFtdcRspInfoField *)msg.ctpMsg;
  TThostFtdcErrorMsgType errormsg;
  utils::gbk2utf8(ctpMsg->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息

  if (ctpMsg->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", ctpMsg->ErrorID, errormsg);
    exit(-1);
  } else {
    auto &marketSer = MarketService::getInstance();

    marketSer.ROLE(publishState).publish_event();

    if (reqInstrumentFrom == "local") {
      marketSer.ROLE(Market).ROLE(CtpMarketApi).reqInstrumentsFromLocal();
    } else if (reqInstrumentFrom == "trader") {
      marketSer.ROLE(Market).ROLE(CtpMarketApi).reqInstrumentsFromTrader();
    } else if (reqInstrumentFrom == "strategy") {
      marketSer.ROLE(Market).ROLE(CtpMarketApi).reqInstrumentsFromStrategy();
    }

    std::string semName = "market_login";
    globalSem.postSemBySemName(semName);
    INFO_LOG("post sem of [%s]", semName.c_str());
  }
}

void CtpEvent::LogoutInfoHandle(MsgStruct &msg) {
  auto ctpMsg = (CThostFtdcRspInfoField *)msg.ctpMsg;
  TThostFtdcErrorMsgType errormsg;
  utils::gbk2utf8(ctpMsg->ErrorMsg, errormsg, sizeof(errormsg));  //报错返回信息

  if (ctpMsg->ErrorID != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", ctpMsg->ErrorID, errormsg);
    exit(-1);
  } else {
    UnSubscribeAllMarketData();
    sleep(WAITTIME_FOR_CONTRACTS_UNSCRIBEED);
    auto &marketSer = MarketService::getInstance();

    if (reqInstrumentFrom == "trader") {
      marketSer.ROLE(loadData).ClassifyContractFiles();
    }

    marketSer.ROLE(publishState).publish_event();

    marketSer.ROLE(Market).release();

    std::string semName = "market_logout";
    globalSem.postSemBySemName(semName);
    INFO_LOG("post sem of [%s]", semName.c_str());
  }
}

void CtpEvent::UnSubscribeAllMarketData(void) {
  int instrumentCount = 0;
  auto &marketSer = MarketService::getInstance();
  vector<utils::InstrumtntID> ins_vec;
  auto iter = marketSer.ROLE(Market).marketApi->md_InstrumentIDs.instrumentIDs.begin();

  while (iter != marketSer.ROLE(Market).marketApi->md_InstrumentIDs.instrumentIDs.end()) {
    ins_vec.push_back(*iter);
    if (ins_vec.size() >= 500) {
      marketSer.ROLE(Market).marketApi->UnSubscribeMarketData(ins_vec);
      ins_vec.clear();
    }
    iter++;
    instrumentCount++;
  }

  if (ins_vec.size() != 0) {
    marketSer.ROLE(Market).marketApi->UnSubscribeMarketData(ins_vec);
    ins_vec.clear();
  }

  INFO_LOG("The number of contracts being unsubscribe is: %d.", instrumentCount);
}

void CtpEvent::set_block_control(ctpview_market::BlockControl_Command command) { block_control = command; }
