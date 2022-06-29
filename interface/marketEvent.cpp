/*
 * marketEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/marketEvent.h"
#include <chrono>
#include <thread>
#include "common/extern/libgo/libgo/libgo.h"
#include "common/extern/log/log.h"
#include "common/self/semaphorePart.h"
#include "market/domain/marketService.h"
#include "market/infra/define.h"
#include "market/infra/recerSender.h"


extern co_chan<MsgStruct> ctpMsgChan;
extern GlobalSem globalSem;
constexpr U32 MAIN_THREAD_WAIT_TIME = 100000;

void MarketEvent::regSessionFunc() {
  int cnt = 0;
  sessionFuncMap.clear();
  sessionFuncMap["market_trader"] = [this](MsgStruct msg) { ROLE(TraderEvent).handle(msg); };
  sessionFuncMap["market_strategy"] = [this](MsgStruct msg) { ROLE(StrategyEvent).handle(msg); };
  sessionFuncMap["market_market"] = [this](MsgStruct msg) { ROLE(SelfEvent).handle(msg); };
  sessionFuncMap["interactor_market"] = [this](MsgStruct msg) { ROLE(InteractEvent).handle(msg); };
  sessionFuncMap["ctp"] = [this](MsgStruct msg) { ROLE(CtpEvent).handle(msg); };
  sessionFuncMap["ctpview_market"] = [this](MsgStruct msg) { ROLE(CtpviewEvent).handle(msg); };

  for (auto &iter : sessionFuncMap) {
    INFO_LOG("sessionFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

bool MarketEvent::init() {
  regSessionFunc();
  ROLE(TraderEvent).init();
  ROLE(StrategyEvent).init();
  ROLE(SelfEvent).init();
  ROLE(InteractEvent).init();
  ROLE(CtpEvent).init();
  ROLE(CtpviewEvent).init();
  return true;
}

bool MarketEvent::run() {
  auto &recerSender = RecerSender::getInstance();

  auto proxyRecRun = [&]() {
    while (1) {
      // INFO_LOG("proxyRecRun while");
      MsgStruct msg = recerSender.ROLE(Recer).ROLE(ProxyRecer).receMsg();
      // INFO_LOG("handle new msg, session is[%s],msgName is[%s]",
      // msg.sessionName.c_str(), msg.msgName.c_str());
      if (!msg.isValid()) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.sessionName.c_str(), msg.msgName.c_str());
        continue;
      }

      if (sessionFuncMap.find(msg.sessionName) != sessionFuncMap.end()) {
        sessionFuncMap[msg.sessionName](msg);
      } else {
        ERROR_LOG("can not find[%s] in sessionFuncMap", msg.sessionName.c_str());
      }
    }
  };
  INFO_LOG("proxyRecRun prepare ok");
  std::thread(proxyRecRun).detach();  // zmq 在libgo的携程里会跑死，单独拎出来一个线程。

  auto ctpRecRun = [&]() {
    MsgStruct msg;
    while (1) {
      ctpMsgChan >> msg;
      if (!msg.isValid()) {
        ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]", msg.sessionName.c_str(), msg.msgName.c_str());
        continue;
      }

      if (sessionFuncMap.find(msg.sessionName) != sessionFuncMap.end()) {
        sessionFuncMap[msg.sessionName](msg);
      } else {
        ERROR_LOG("can not find[%s] in sessionFuncMap", msg.sessionName.c_str());
      }

      globalSem.postSemBySemName(msg.msgName);
    }
  };
  INFO_LOG("ctpRecRun prepare ok");
  std::thread(ctpRecRun).detach();

  while (1) {
    usleep(MAIN_THREAD_WAIT_TIME);
  }
  return true;
}
