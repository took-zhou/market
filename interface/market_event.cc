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
  auto &recer_sender = RecerSender::GetInstance();

  auto proxy_rec_run = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (recer_sender.ROLE(Recer).ROLE(ProxyRecer).ReceMsg(msg) == false) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.session_name.c_str(), msg.msg_name.c_str());
        continue;
      }

      if (session_func_map.find(msg.session_name) != session_func_map.end()) {
        session_func_map[msg.session_name](msg);
      } else {
        ERROR_LOG("can not find[%s] in session_func_map", msg.session_name.c_str());
      }
    }
  };
  INFO_LOG("proxyRecRun prepare ok");
  std::thread(proxy_rec_run).detach();

  auto itp_rec_run = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (recer_sender.ROLE(Recer).ROLE(ItpRecer).ReceMsg(msg) == false) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.session_name.c_str(), msg.msg_name.c_str());
        GlobalSem::GetInstance().PostSemBySemName(GlobalSem::kApiRecv);
        continue;
      }

      if (session_func_map.find(msg.session_name) != session_func_map.end()) {
        session_func_map[msg.session_name](msg);
      } else {
        ERROR_LOG("can not find[%s] in session_func_map", msg.session_name.c_str());
      }

      GlobalSem::GetInstance().PostSemBySemName(GlobalSem::kApiRecv);
    }
  };
  INFO_LOG("itpRecRun prepare ok");
  std::thread(itp_rec_run).detach();

  while (1) {
    std::this_thread::sleep_for(1000ms);
  }
  return true;
}
