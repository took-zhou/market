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
#include "market/infra/recer_sender.h"

void MarketEvent::RegSessionFunc() {
  session_func_map_.clear();
  session_func_map_["market_trader"] = [this](utils::ItpMsg msg) { ROLE(TraderEvent).Handle(msg); };
  session_func_map_["strategy_market"] = [this](utils::ItpMsg msg) { ROLE(StrategyEvent).Handle(msg); };
  session_func_map_["btp_market"] = [this](utils::ItpMsg msg) { ROLE(BtpEvent).Handle(msg); };
  session_func_map_["ctp_market"] = [this](utils::ItpMsg msg) { ROLE(CtpEvent).Handle(msg); };
  session_func_map_["xtp_market"] = [this](utils::ItpMsg msg) { ROLE(XtpEvent).Handle(msg); };
  session_func_map_["otp_market"] = [this](utils::ItpMsg msg) { ROLE(OtpEvent).Handle(msg); };
  session_func_map_["ftp_market"] = [this](utils::ItpMsg msg) { ROLE(FtpEvent).Handle(msg); };
  session_func_map_["gtp_market"] = [this](utils::ItpMsg msg) { ROLE(GtpEvent).Handle(msg); };
  session_func_map_["ctpview_market"] = [this](utils::ItpMsg msg) { ROLE(CtpviewEvent).Handle(msg); };
  session_func_map_["market_market"] = [this](utils::ItpMsg msg) { ROLE(SelfEvent).Handle(msg); };
}

MarketEvent::MarketEvent() { RegSessionFunc(); }

MarketEvent::~MarketEvent() {
  running_ = false;
  if (proxy_rec_thread_.joinable()) {
    proxy_rec_thread_.join();
    INFO_LOG("proxy rec thread exit");
  }
  if (itp_rec_thread_.joinable()) {
    itp_rec_thread_.join();
    INFO_LOG("itp rec thread exit");
  }
}

bool MarketEvent::Run() {
  running_ = true;

  proxy_rec_thread_ = std::thread(&MarketEvent::ProxyRecTask, this);
  INFO_LOG("proxy rec thread start");

  itp_rec_thread_ = std::thread(&MarketEvent::ItpRecTask, this);
  INFO_LOG("itp rec thread start");

  return true;
}

void MarketEvent::ProxyRecTask() {
  utils::ItpMsg msg;
  auto &recer_sender = RecerSender::GetInstance();
  while (running_) {
    if (!recer_sender.ROLE(Recer).ROLE(ProxyRecer).ReceMsg(msg)) {
      continue;
    }

    if (session_func_map_.find(msg.session_name) != session_func_map_.end()) {
      session_func_map_[msg.session_name](msg);
    } else {
      ERROR_LOG("can not find[%s] in session_func_map_", msg.session_name.c_str());
    }
  }
}

void MarketEvent::ItpRecTask() {
  utils::ItpMsg msg;
  auto &recer_sender = RecerSender::GetInstance();
  while (running_) {
    if (!recer_sender.ROLE(Recer).ROLE(InnerRecer).ReceMsg(msg)) {
      continue;
    }

    if (session_func_map_.find(msg.session_name) != session_func_map_.end()) {
      session_func_map_[msg.session_name](msg);
    } else {
      ERROR_LOG("can not find[%s] in session_func_map_", msg.session_name.c_str());
    }

    GlobalSem::GetInstance().PostSemBySemName(SemName::kApiRecv);
  }
}
