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
  topic_list.push_back("strategy_market.InstrumentReq");
  topic_list.push_back("strategy_market.MarketStateRsp");
  topic_list.push_back("strategy_market.InsertControlParaReq");
  topic_list.push_back("strategy_market.EraseControlParaReq");
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

  auto &zmq_base = BaseZmq::GetInstance();
  for (auto &topic : topic_list) {
    zmq_base.SubscribeTopic(topic.c_str());
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
  auto &zmq_base = BaseZmq::GetInstance();

  std::string recv_string;
  recv_string.resize(256);

  int msgsize = zmq_base.RecvMsg(recv_string);
  if (msgsize != -1) {
    int start_index = 0;
    int seg_index = 0;

    for (int i = 0; i < msgsize; i++) {
      if (recv_string[i] == '.') {
        if (seg_index == 0) {
          msg.session_name.resize(i - start_index);
          memcpy(&msg.session_name[0], &recv_string[start_index], (i - start_index));
        } else if (seg_index == 1) {
          msg.msg_name.resize(i - start_index);
          memcpy(&msg.msg_name[0], &recv_string[start_index], (i - start_index));
        }
        start_index = i + 1;
        seg_index++;
      } else if (recv_string[i] == ' ') {
        if (seg_index == 1) {
          i = i;
          msg.msg_name.resize(i - start_index);
          memcpy(&msg.msg_name[0], &recv_string[start_index], (i - start_index));
        }
        start_index = i + 1;
        break;
      }
    }

    msg.pb_msg.resize(msgsize - start_index);
    memcpy(&msg.pb_msg[0], &recv_string[start_index], (msgsize - start_index));
  } else {
    out = false;
  }

  return out;
}
