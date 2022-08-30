/*
 * XtpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/xtpEvent/xtpEvent.h"
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

XtpEvent::XtpEvent() {
  regMsgFun();
  auto &jsonCfg = utils::JsonConfig::getInstance();
  reqInstrumentFrom = jsonCfg.getConfig("market", "SubscribeMarketDataFrom").get<std::string>();
  INFO_LOG("SubscribeMarketDataFrom: %s.", reqInstrumentFrom.c_str());
}

void XtpEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["OnDepthMarketData"] = [this](utils::ItpMsg &msg) { OnDepthMarketDataHandle(msg); };
  msgFuncMap["OnRspUserLogin"] = [this](utils::ItpMsg &msg) { OnRspUserLoginHandle(msg); };
  msgFuncMap["OnRspUserLogout"] = [this](utils::ItpMsg &msg) { OnRspUserLogoutHandle(msg); };
  msgFuncMap["OnQueryAllTickers"] = [this](utils::ItpMsg &msg) { OnQueryAllTickersHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void XtpEvent::handle(utils::ItpMsg &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void XtpEvent::OnDepthMarketDataHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto deepdata = reinterpret_cast<XTPMD *>(itp_msg.address());
  auto &marketSer = MarketService::getInstance();

  if (reqInstrumentFrom == "api") {
    marketSer.ROLE(loadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control == ctpview_market::BlockControl_Command_unblock) {
      marketSer.ROLE(publishData).directForwardDataToStrategy(deepdata);
    }
  }
}

void XtpEvent::OnRspUserLoginHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto xtpri = reinterpret_cast<XTPRI *>(itp_msg.address());
  if (xtpri->error_id != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", xtpri->error_id, xtpri->error_msg);
    exit(-1);
  } else {
    auto &marketSer = MarketService::getInstance();
    // marketSer.ROLE(publishState).publish_event();

    if (reqInstrumentFrom == "local") {
      marketSer.ROLE(subscribeManager).reqInstrumentsFromLocal();
    } else if (reqInstrumentFrom == "api") {
      INFO_LOG("reqInstrumentsFromMarket");
      marketSer.ROLE(subscribeManager).reqInstrumentsFromMarket();
    } else if (reqInstrumentFrom == "strategy") {
      marketSer.ROLE(subscribeManager).reqInstrumrntFromControlPara();
    }
  }
}

void XtpEvent::OnRspUserLogoutHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();

  auto xtpri = reinterpret_cast<XTPRI *>(itp_msg.address());
  if (xtpri->error_id != 0) {
    // 端登失败，客户端需进行错误处理
    ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", xtpri->error_id, xtpri->error_msg);
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

void XtpEvent::OnQueryAllTickersHandle(utils::ItpMsg &msg) {
  ipc::message itpMsg;
  itpMsg.ParseFromString(msg.pbMsg);
  auto &itp_msg = itpMsg.itp_msg();
  auto xtpqsi = reinterpret_cast<XTPQSI *>(itp_msg.address());
  static int instrumentCount;
  static vector<utils::InstrumtntID> ins_vec;
  auto &marketServer = MarketService::getInstance();

  if (!itp_msg.is_last()) {
    utils::InstrumtntID instrumtntID;
    if (xtpqsi->exchange_id == XTP_EXCHANGE_SH) {
      instrumtntID.exch = "SHSE";
      instrumtntID.ins = xtpqsi->ticker;
    } else if (xtpqsi->exchange_id == XTP_EXCHANGE_SZ) {
      instrumtntID.exch = "SZSE";
      instrumtntID.ins = xtpqsi->ticker;
    } else {
      return;
    }

    if (instrumtntID.ins.find(" ") == instrumtntID.ins.npos) {
      auto &marketSer = MarketService::getInstance();
      marketSer.ROLE(loadData).insertInsExchPair(instrumtntID.ins, instrumtntID.exch);
      ins_vec.push_back(instrumtntID);

      instrumentCount++;
    }

    if (ins_vec.size() >= 500) {
      INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
      marketServer.ROLE(subscribeManager).subscribeInstrument(ins_vec);
      ins_vec.clear();
    }
  } else {
    if (instrumentCount > 0) {
      marketServer.ROLE(subscribeManager).subscribeInstrument(ins_vec);
      INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
      instrumentCount = 0;
      ins_vec.clear();
    }
  }
}

void XtpEvent::set_block_control(ctpview_market::BlockControl_Command command) { block_control = command; }
