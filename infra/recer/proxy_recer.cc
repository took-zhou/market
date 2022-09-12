/*
 * proxyRecer.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include <map>

#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "market/infra/base_zmq.h"
#include "market/infra/recer/proxy_recer.h"

ProxyRecer::ProxyRecer() {
  topic_list.clear();

  // strategy_market
  topic_list.push_back("strategy_market.TickSubscribeReq");
  topic_list.push_back("strategy_market.TickStartStopIndication");
  topic_list.push_back("strategy_market.ActiveSafetyRsp");

  // market_market
  topic_list.push_back("market_market.HeartBeat");

  // market_trader
  topic_list.push_back("market_trader.QryInstrumentRsp");

  topic_list.push_back("manage_market.TickMarketStateReq");

  topic_list.push_back("ctpview_market.LoginControl");
  topic_list.push_back("ctpview_market.CheckStrategyAlive");
  topic_list.push_back("ctpview_market.BlockControl");
  topic_list.push_back("ctpview_market.BugInjection");
  topic_list.push_back("ctpview_market.SimulateMarketState");

  auto &zmqBase = BaseZmq::getInstance();
  for (auto &topic : topic_list) {
    zmqBase.SubscribeTopic(topic.c_str());
  }
}

bool ProxyRecer::IsTopicInSubTopics(std::string title) {
  for (auto &topic : topic_list) {
    if (topic == title) {
      return true;
    }
  }
  return false;
}

bool ProxyRecer::ReceMsg(utils::ItpMsg &msg) {
  bool out = true;
  auto &zmqBase = BaseZmq::getInstance();

  std::string recvString;
  recvString.resize(256);

  int msgsize = zmqBase.RecvMsg(recvString);
  if (msgsize != -1) {
    int startIndex = 0;
    int segIndex = 0;

    for (int i = 0; i < msgsize; i++) {
      if (recvString[i] == '.') {
        if (segIndex == 0) {
          msg.sessionName.resize(i - startIndex);
          memcpy(&msg.sessionName[0], &recvString[startIndex], (i - startIndex));
        } else if (segIndex == 1) {
          msg.msgName.resize(i - startIndex);
          memcpy(&msg.msgName[0], &recvString[startIndex], (i - startIndex));
        }
        startIndex = i + 1;
        segIndex++;
      } else if (recvString[i] == ' ') {
        if (segIndex == 1) {
          i = i;
          msg.msgName.resize(i - startIndex);
          memcpy(&msg.msgName[0], &recvString[startIndex], (i - startIndex));
        }
        startIndex = i + 1;
        break;
      }
    }

    msg.pbMsg.resize(msgsize - startIndex);
    memcpy(&msg.pbMsg[0], &recvString[startIndex], (msgsize - startIndex));
  } else {
    out = false;
  }

  return out;
}
