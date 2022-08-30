/*
 * marketEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/marketEvent.h"
#include <chrono>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/domain/marketService.h"
#include "market/infra/recerSender.h"

void MarketEvent::regSessionFunc() {
  int cnt = 0;
  sessionFuncMap.clear();
  sessionFuncMap["market_trader"] = [this](utils::ItpMsg msg) { ROLE(TraderEvent).handle(msg); };
  sessionFuncMap["strategy_market"] = [this](utils::ItpMsg msg) { ROLE(StrategyEvent).handle(msg); };
  sessionFuncMap["manage_market"] = [this](utils::ItpMsg msg) { ROLE(ManageEvent).handle(msg); };
  sessionFuncMap["ctp_market"] = [this](utils::ItpMsg msg) { ROLE(CtpEvent).handle(msg); };
  sessionFuncMap["xtp_market"] = [this](utils::ItpMsg msg) { ROLE(XtpEvent).handle(msg); };
  sessionFuncMap["ctpview_market"] = [this](utils::ItpMsg msg) { ROLE(CtpviewEvent).handle(msg); };

  for (auto &iter : sessionFuncMap) {
    INFO_LOG("sessionFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

MarketEvent::MarketEvent() { regSessionFunc(); }

bool MarketEvent::run() {
  auto &recerSender = RecerSender::getInstance();

  auto proxyRecRun = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (recerSender.ROLE(Recer).ROLE(ProxyRecer).receMsg(msg) == false) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.sessionName.c_str(), msg.msgName.c_str());
        continue;
      }

      INFO_LOG("proxyRecRun");
      if (sessionFuncMap.find(msg.sessionName) != sessionFuncMap.end()) {
        sessionFuncMap[msg.sessionName](msg);
      } else {
        ERROR_LOG("can not find[%s] in sessionFuncMap", msg.sessionName.c_str());
      }
    }
  };
  INFO_LOG("proxyRecRun prepare ok");
  std::thread(proxyRecRun).detach();

  auto itpRecRun = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (recerSender.ROLE(Recer).ROLE(ItpRecer).receMsg(msg) == false) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.sessionName.c_str(), msg.msgName.c_str());
        GlobalSem::getInstance().postSemBySemName(GlobalSem::apiRecv);
        continue;
      }

      if (sessionFuncMap.find(msg.sessionName) != sessionFuncMap.end()) {
        sessionFuncMap[msg.sessionName](msg);
      } else {
        ERROR_LOG("can not find[%s] in sessionFuncMap", msg.sessionName.c_str());
      }

      GlobalSem::getInstance().postSemBySemName(GlobalSem::apiRecv);
    }
  };
  INFO_LOG("itpRecRun prepare ok");
  std::thread(itpRecRun).detach();

  while (1) {
    std::this_thread::sleep_for(1000ms);
  }
  return true;
}
