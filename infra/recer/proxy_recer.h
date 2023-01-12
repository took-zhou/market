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
  bool ReceMsg(utils::ItpMsg &msg);
  bool IsTopicInSubTopics(std::string title);
  std::vector<std::string> topic_list;

 private:
  void *receiver_{nullptr};
  void SubscribeTopic();
  void UnSubscribeTopic();
};

#endif /* WORKSPACE_MARKET_INFRA_PROXYRECER_H_ */
