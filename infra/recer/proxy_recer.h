/*
 * proxyRecer.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_PROXYRECER_H_
#define WORKSPACE_MARKET_INFRA_PROXYRECER_H_

#include <vector>
#include "common/self/utils.h"

struct ProxyRecer {
  ProxyRecer();
  ~ProxyRecer();
  bool ReceMsg(utils::ItpMsg &msg);
  bool IsTopicInSubTopics(const std::string &title);

 private:
  void SubscribeTopic();
  void UnSubscribeTopic();
  std::vector<std::string> topic_list_;
  void *receiver_{nullptr};
  const int rec_timeout_ = 1000;
  std::string sub_ipaddport_;
};

#endif /* WORKSPACE_MARKET_INFRA_PROXYRECER_H_ */
