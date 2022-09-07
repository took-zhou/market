/*
 * zmqBase.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INFRA_ZMQBASE_H_
#define WORKSPACE_MARKET_INFRA_ZMQBASE_H_

#include <string>
#include "common/extern/libzmq/include/zmq.h"

struct ZmqBase {
  ZmqBase();
  ZmqBase(const ZmqBase &) = delete;
  ZmqBase &operator=(const ZmqBase &) = delete;
  static ZmqBase &getInstance() {
    static ZmqBase instance;
    return instance;
  }

  void SubscribeTopic(const char *topicStr);
  void unSubscribeTopic(const char *topicStr);

  int SendMsg(const std::string &msg);
  int RecvMsg(std::string &msg);
  void *context{nullptr};
  void *receiver{nullptr};
  void *publisher{nullptr};
};

#endif /* WORKSPACE_MARKET_INFRA_ZMQBASE_H_ */
