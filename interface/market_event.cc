/*
 * marketEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/market_event.h"
#include <chrono>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

void MarketEvent::RegSessionFunc() {
  int cnt = 0;
  session_func_map.clear();
  session_func_map["market_trader"] = [this](utils::ItpMsg msg) { ROLE(TraderEvent).Handle(msg); };
  session_func_map["strategy_market"] = [this](utils::ItpMsg msg) { ROLE(StrategyEvent).Handle(msg); };
  session_func_map["manage_market"] = [this](utils::ItpMsg msg) { ROLE(ManageEvent).Handle(msg); };
  session_func_map["ctp_market"] = [this](utils::ItpMsg msg) { ROLE(CtpEvent).Handle(msg); };
  session_func_map["xtp_market"] = [this](utils::ItpMsg msg) { ROLE(XtpEvent).Handle(msg); };
  session_func_map["ctpview_market"] = [this](utils::ItpMsg msg) { ROLE(CtpviewEvent).Handle(msg); };

  for (auto &iter : session_func_map) {
    INFO_LOG("session_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

MarketEvent::MarketEvent() { RegSessionFunc(); }

bool MarketEvent::Run() {
  auto &recerSender = RecerSender::getInstance();

  auto proxyRecRun = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (recerSender.ROLE(Recer).ROLE(ProxyRecer).ReceMsg(msg) == false) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.sessionName.c_str(), msg.msgName.c_str());
        continue;
      }

      if (session_func_map.find(msg.sessionName) != session_func_map.end()) {
        session_func_map[msg.sessionName](msg);
      } else {
        ERROR_LOG("can not find[%s] in session_func_map", msg.sessionName.c_str());
      }
    }
  };
  INFO_LOG("proxyRecRun prepare ok");
  std::thread(proxyRecRun).detach();

  auto itpRecRun = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (recerSender.ROLE(Recer).ROLE(ItpRecer).ReceMsg(msg) == false) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.sessionName.c_str(), msg.msgName.c_str());
        GlobalSem::getInstance().PostSemBySemName(GlobalSem::kApiRecv);
        continue;
      }

      if (session_func_map.find(msg.sessionName) != session_func_map.end()) {
        session_func_map[msg.sessionName](msg);
      } else {
        ERROR_LOG("can not find[%s] in session_func_map", msg.sessionName.c_str());
      }

      GlobalSem::getInstance().PostSemBySemName(GlobalSem::kApiRecv);
    }
  };
  INFO_LOG("itpRecRun prepare ok");
  std::thread(itpRecRun).detach();

  while (1) {
    std::this_thread::sleep_for(1000ms);
  }
  return true;
}
