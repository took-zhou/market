/*
 * proxyRecer.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/infra/recer/proxy_recer.h"
#include <map>
#include <thread>
#include "common/extern/libzmq/include/zmq.h"
#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "market/infra/base_zmq.h"

ProxyRecer::ProxyRecer() {
  receiver_ = zmq_socket(BaseZmq::GetInstance().GetContext(), ZMQ_SUB);
  zmq_setsockopt(receiver_, ZMQ_RCVTIMEO, &rec_timeout_, sizeof(rec_timeout_));

  sub_ipaddport_ = "tcp://" + BaseZmq::GetInstance().GetLocalIp() + ":8100";
  int result = zmq_connect(receiver_, sub_ipaddport_.c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (result != 0) {
    ERROR_LOG("receiver connect to %s failed", sub_ipaddport_.c_str());
  } else {
    INFO_LOG("receiver connect to %s ok", sub_ipaddport_.c_str());
  }

  SubscribeTopic();
}

ProxyRecer::~ProxyRecer() {
  zmq_close(receiver_);
  INFO_LOG("receiver disconnect to %s ok", sub_ipaddport_.c_str());
}

bool ProxyRecer::IsTopicInSubTopics(const std::string &title) {
  for (auto &topic : topic_list_) {
    if (topic == title) {
      return true;
    }
  }
  return false;
}

void ProxyRecer::SubscribeTopic() {
  topic_list_.clear();

  // strategy_market
  topic_list_.push_back("strategy_market.TickSubscribeReq");
  topic_list_.push_back("strategy_market.TickStartStopIndication");
  topic_list_.push_back("strategy_market.InstrumentReq");
  topic_list_.push_back("strategy_market.MarketStateRsp");
  topic_list_.push_back("strategy_market.CheckMarketAliveReq");

  // market_trader
  topic_list_.push_back("market_trader.QryInstrumentRsp");
  topic_list_.push_back("market_trader.MarketStateRsp");

  // ctpview_market
  topic_list_.push_back("ctpview_market.LoginControl");
  topic_list_.push_back("ctpview_market.BlockControl");
  topic_list_.push_back("ctpview_market.BugInjection");
  topic_list_.push_back("ctpview_market.ProfilerControl");
  topic_list_.push_back("ctpview_market.UpdatePara");
  topic_list_.push_back("ctpview_market.ClearDiagnosticEvent");
  topic_list_.push_back("ctpview_market.SendTestEmail");

  // market_market
  topic_list_.push_back("market_market.SendEmail");

  for (auto &topic : topic_list_) {
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
