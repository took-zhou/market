/*
 * base_zmq.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INFRA_ZMQBASE_H_
#define WORKSPACE_MARKET_INFRA_ZMQBASE_H_

#include <string>
#include "common/extern/libzmq/include/zmq.h"

struct BaseZmq {
  BaseZmq();
  BaseZmq(const BaseZmq &) = delete;
  BaseZmq &operator=(const BaseZmq &) = delete;
  static BaseZmq &GetInstance() {
    static BaseZmq instance;
    return instance;
  }

  void SubscribeTopic(const char *topic_str);
  void UnSubscribeTopic(const char *topic_str);

  int SendMsg(const std::string &msg);
  int RecvMsg(std::string &msg);
  void *context{nullptr};
  void *receiver{nullptr};
  void *publisher{nullptr};
};

#endif /* WORKSPACE_MARKET_INFRA_ZMQBASE_H_ */
