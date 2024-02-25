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
#include "common/self/global_sem.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

void MarketEvent::RegSessionFunc() {
  int cnt = 0;
  session_func_map_.clear();
  session_func_map_["market_trader"] = [this](utils::ItpMsg msg) { ROLE(TraderEvent).Handle(msg); };
  session_func_map_["strategy_market"] = [this](utils::ItpMsg msg) { ROLE(StrategyEvent).Handle(msg); };
  session_func_map_["btp_market"] = [this](utils::ItpMsg msg) { ROLE(BtpEvent).Handle(msg); };
  session_func_map_["ctp_market"] = [this](utils::ItpMsg msg) { ROLE(CtpEvent).Handle(msg); };
  session_func_map_["xtp_market"] = [this](utils::ItpMsg msg) { ROLE(XtpEvent).Handle(msg); };
  session_func_map_["otp_market"] = [this](utils::ItpMsg msg) { ROLE(OtpEvent).Handle(msg); };
  session_func_map_["ftp_market"] = [this](utils::ItpMsg msg) { ROLE(FtpEvent).Handle(msg); };
  session_func_map_["ctpview_market"] = [this](utils::ItpMsg msg) { ROLE(CtpviewEvent).Handle(msg); };

  for (auto &iter : session_func_map_) {
    INFO_LOG("session_func_map_[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

MarketEvent::MarketEvent() { RegSessionFunc(); }

bool MarketEvent::Run() {
  auto &recer_sender = RecerSender::GetInstance();

  auto proxy_rec_run = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (!recer_sender.ROLE(Recer).ROLE(ProxyRecer).ReceMsg(msg)) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.session_name.c_str(), msg.msg_name.c_str());
        continue;
      }

      if (session_func_map_.find(msg.session_name) != session_func_map_.end()) {
        session_func_map_[msg.session_name](msg);
      } else {
        ERROR_LOG("can not find[%s] in session_func_map_", msg.session_name.c_str());
      }
    }
  };
  INFO_LOG("proxyRecRun prepare ok");
  std::thread(proxy_rec_run).detach();

  auto itp_rec_run = [&]() {
    utils::ItpMsg msg;
    while (1) {
      if (!recer_sender.ROLE(Recer).ROLE(InnerRecer).ReceMsg(msg)) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.session_name.c_str(), msg.msg_name.c_str());
        GlobalSem::GetInstance().PostSemBySemName(SemName::kApiRecv);
        continue;
      }

      if (session_func_map_.find(msg.session_name) != session_func_map_.end()) {
        session_func_map_[msg.session_name](msg);
      } else {
        ERROR_LOG("can not find[%s] in session_func_map_", msg.session_name.c_str());
      }

      GlobalSem::GetInstance().PostSemBySemName(SemName::kApiRecv);
    }
  };
  INFO_LOG("itpRecRun prepare ok");
  std::thread(itp_rec_run).detach();

  recer_sender.Run();
  return true;
}
