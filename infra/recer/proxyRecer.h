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
  bool receMsg(utils::ItpMsg &msg);
  bool isTopicInSubTopics(std::string title);
  std::vector<std::string> topicList;
};

#endif /* WORKSPACE_MARKET_INFRA_PROXYRECER_H_ */
