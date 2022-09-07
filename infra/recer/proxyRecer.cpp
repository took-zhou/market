/*
 * proxyRecer.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include <map>

#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "market/infra/recer/proxyRecer.h"
#include "market/infra/zmqBase.h"

ProxyRecer::ProxyRecer() {
  topicList.clear();

  // strategy_market
  topicList.push_back("strategy_market.TickSubscribeReq");
  topicList.push_back("strategy_market.TickStartStopIndication");
  topicList.push_back("strategy_market.ActiveSafetyRsp");

  // market_market
  topicList.push_back("market_market.HeartBeat");

  // market_trader
  topicList.push_back("market_trader.QryInstrumentRsp");

  topicList.push_back("manage_market.TickMarketStateReq");

  topicList.push_back("ctpview_market.LoginControl");
  topicList.push_back("ctpview_market.CheckStrategyAlive");
  topicList.push_back("ctpview_market.BlockControl");
  topicList.push_back("ctpview_market.BugInjection");
  topicList.push_back("ctpview_market.SimulateMarketState");

  auto &zmqBase = ZmqBase::getInstance();
  for (auto &topic : topicList) {
    zmqBase.SubscribeTopic(topic.c_str());
  }
}

bool ProxyRecer::isTopicInSubTopics(std::string title) {
  for (auto &topic : topicList) {
    if (topic == title) {
      return true;
    }
  }
  return false;
}

bool ProxyRecer::receMsg(utils::ItpMsg &msg) {
  bool out = true;
  auto &zmqBase = ZmqBase::getInstance();

  std::string recvString;
  recvString.resize(256);

  int msgsize = zmqBase.RecvMsg(recvString);
  if (msgsize != -1) {
    int index = 0;
    int segIndex = 0;
    char temp[msgsize];

    for (int i = 0; i < msgsize; i++) {
      temp[index] = recvString[i];
      if (temp[index] == '.') {
        if (segIndex == 0) {
          temp[index] = '\0';
          msg.sessionName = temp;
        } else if (segIndex == 1) {
          temp[index] = '\0';
          msg.msgName = temp;
        }
        segIndex++;
        index = 0;
      } else if (temp[index] == ' ') {
        if (segIndex == 1) {
          temp[index] = '\0';
          msg.msgName = temp;
        }
        segIndex++;
        index = 0;
      } else {
        index++;
      }
    }
    msg.pbMsg.resize(index);
    memcpy(&msg.pbMsg[0], temp, index);
  } else {
    out = false;
  }

  return out;
}
