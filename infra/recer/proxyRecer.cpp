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

  char *recContent = zmqBase.RecvMsg();
  if (recContent != nullptr) {
    int index = 0;
    int segIndex = 0;
    int length = strlen(recContent) + 1;
    char temp[length];
    for (int i = 0; i < length; i++) {
      temp[index] = recContent[i];
      if (recContent[i] == '.' && segIndex == 0) {
        temp[index] = '\0';
        msg.sessionName = temp;
        index = 0;
        segIndex++;
      } else if (recContent[i] == ' ' && segIndex == 1) {
        temp[index] = '\0';
        msg.msgName = temp;
        index = 0;
        segIndex++;
      } else if (recContent[i] == '\0' && segIndex == 2) {
        msg.pbMsg = temp;
        break;
      } else {
        index++;
      }
    }
  } else {
    out = false;
  }

  return out;
}
