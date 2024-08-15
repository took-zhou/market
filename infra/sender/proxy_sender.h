/*
 * proxy_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_PROXYSENDER_H_
#define WORKSPACE_MARKET_INFRA_PROXYSENDER_H_

#include <mutex>
#include <string>
#include "common/self/utils.h"

struct ProxySender {
 public:
  ProxySender();
  ~ProxySender();
  bool SendMsg(utils::ItpMsg &msg);

 private:
  void *publisher_{nullptr};
  std::mutex m_lock_;
  std::string pub_ipaddport_;
};

#endif /* WORKSPACE_MARKET_INFRA_PROXYSENDER_H_ */
