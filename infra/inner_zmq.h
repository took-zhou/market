/*
 * inner_zmq.h
 *
 *  Created on: 2021年2月17日
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_INNERZMQBASE_H_
#define WORKSPACE_MARKET_INFRA_INNERZMQBASE_H_
#include <string>

#include "common/extern/libzmq/include/zmq.h"
#include "common/self/utils.h"

struct InnerZmq {
  InnerZmq();
  InnerZmq(const InnerZmq &) = delete;
  InnerZmq &operator=(const InnerZmq &) = delete;
  static InnerZmq &getInstance() {
    static InnerZmq instance;
    return instance;
  }

  int PushTask(const utils::ItpMsg &msg);
  int PullTask(utils::ItpMsg &msg);
  std::string inproc_address{"inproc://innerzmq"};
  void *context{nullptr};
  void *receiver{nullptr};
  void *sender{nullptr};
};

#endif /* WORKSPACE_MARKET_INFRA_INNERZMQBASE_H_ */
