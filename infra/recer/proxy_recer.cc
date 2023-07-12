/*
 * proxyRecer.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/infra/recer/proxy_recer.h"
#include <map>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "market/infra/base_zmq.h"

ProxyRecer::ProxyRecer() {
  receiver_ = zmq_socket(BaseZmq::GetInstance().context, ZMQ_SUB);

  string sub_ipaddport = "tcp://" + BaseZmq::GetInstance().local_ip + ":8100";
  int result = zmq_connect(receiver_, sub_ipaddport.c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (result != 0) {
    ERROR_LOG("receiver_ connect to %s failed", sub_ipaddport.c_str());
  } else {
    INFO_LOG("receiver_ connect to %s ok", sub_ipaddport.c_str());
  }

  SubscribeTopic();
}

bool ProxyRecer::IsTopicInSubTopics(std::string title) {
  for (auto &topic : topic_list) {
    if (topic == title) {
      return true;
    }
  }
  return false;
}

void ProxyRecer::SubscribeTopic() {
  topic_list.clear();

  // strategy_market
  topic_list.push_back("strategy_market.TickSubscribeReq");
  topic_list.push_back("strategy_market.TickStartStopIndication");
  topic_list.push_back("strategy_market.InstrumentReq");
  topic_list.push_back("strategy_market.MarketStateRsp");
  // market_market
  topic_list.push_back("market_market.HeartBeat");

  // market_trader
  topic_list.push_back("market_trader.QryInstrumentRsp");

  topic_list.push_back("ctpview_market.LoginControl");
  topic_list.push_back("ctpview_market.CheckStrategyAlive");
  topic_list.push_back("ctpview_market.BlockControl");
  topic_list.push_back("ctpview_market.BugInjection");
  topic_list.push_back("ctpview_market.SimulateMarketState");
  topic_list.push_back("ctpview_market.BackTestControl");
  topic_list.push_back("ctpview_market.TickStartStopIndication");
  topic_list.push_back("ctpview_market.ProfilerControl");
  topic_list.push_back("ctpview_market.UpdatePara");

  for (auto &topic : topic_list) {
    INFO_LOG("%s", topic.c_str());
    zmq_setsockopt(receiver_, ZMQ_SUBSCRIBE, topic.c_str(), strlen(topic.c_str()));
  }
}

void ProxyRecer::UnSubscribeTopic() {}

bool ProxyRecer::ReceMsg(utils::ItpMsg &msg) {
  std::string recv_string;
  recv_string.resize(256);

  int msgsize = zmq_recv(receiver_, &recv_string[0], recv_string.length() - 1, 0);
  return utils::ReceMsg(msg, recv_string, msgsize);
}
