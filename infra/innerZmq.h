/*
 * innerZmq.h
 *
 *  Created on: 2021年2月17日
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_INNERZMQBASE_H_
#define WORKSPACE_MARKET_INFRA_INNERZMQBASE_H_
#include <sstream>
#include <string>

#include "common/extern/libzmq/include/zmq.h"

struct InnerZmq {
  InnerZmq();
  InnerZmq(const InnerZmq &) = delete;
  InnerZmq &operator=(const InnerZmq &) = delete;
  static InnerZmq &getInstance() {
    static InnerZmq instance;
    return instance;
  }

  int pushTask(const char *head, const char *msg);
  char *pullTask();
  std::string inprocAddress{"inproc://innerzmq"};
  void *context{nullptr};
  void *receiver{nullptr};
  void *sender{nullptr};
};

#endif /* WORKSPACE_MARKET_INFRA_INNERZMQBASE_H_ */
