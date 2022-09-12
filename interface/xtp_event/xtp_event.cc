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
  auto &jsonCfg = utils::JsonConfig::getInstance();
  req_instrument_from = jsonCfg.get_config("market", "SubscribeMarketDataFrom").get<std::string>();
  INFO_LOG("SubscribeMarketDataFrom: %s.", req_instrument_from.c_str());
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
  auto iter = msg_func_map.find(msg.msgName);
  if (iter != msg_func_map.end()) {
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

  if (req_instrument_from == "api") {
    marketSer.ROLE(LoadData).LoadDepthMarketDataToCsv(deepdata);
  } else {
    if (block_control == ctpview_market::BlockControl_Command_unblock) {
      marketSer.ROLE(PublishData).DirectForwardDataToStrategy(deepdata);
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
    marketSer.ROLE(PublishState).publish_event();

    if (req_instrument_from == "local") {
      marketSer.ROLE(SubscribeManager).reqInstrumentsFromLocal();
    } else if (req_instrument_from == "api") {
      INFO_LOG("reqInstrumentsFromMarket");
      marketSer.ROLE(SubscribeManager).reqInstrumentsFromMarket();
    } else if (req_instrument_from == "strategy") {
      marketSer.ROLE(SubscribeManager).reqInstrumrntFromControlPara();
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
    marketSer.ROLE(SubscribeManager).unSubscribeAll();

    std::this_thread::sleep_for(1000ms);

    if (req_instrument_from == "trader" && marketSer.ROLE(MarketTimeState).get_time_state() == kLogoutTime) {
      marketSer.ROLE(LoadData).ClassifyContractFiles();
    }

    marketSer.ROLE(LoadData).ClearInsExchPair();

    marketSer.ROLE(PublishState).publish_event();
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
      marketSer.ROLE(LoadData).InsertInsExchPair(instrumtntID.ins, instrumtntID.exch);
      ins_vec.push_back(instrumtntID);

      instrumentCount++;
    }

    if (ins_vec.size() >= 500) {
      INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
      marketServer.ROLE(SubscribeManager).subscribeInstrument(ins_vec);
      ins_vec.clear();
    }
  } else {
    if (instrumentCount > 0) {
      marketServer.ROLE(SubscribeManager).subscribeInstrument(ins_vec);
      INFO_LOG("The number of trading contracts is: %d.", instrumentCount);
      instrumentCount = 0;
      ins_vec.clear();
    }
  }
}

void XtpEvent::set_block_control(ctpview_market::BlockControl_Command command) { block_control = command; }
