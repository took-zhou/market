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
 public:
  BaseZmq();
  BaseZmq(const BaseZmq &) = delete;
  BaseZmq &operator=(const BaseZmq &) = delete;
  static BaseZmq &GetInstance() {
    static BaseZmq instance;
    return instance;
  }

  std::string &GetLocalIp();
  std::string &GetInprocAddress();
  void *GetContext();

 private:
  std::string local_ip_{""};
  std::string inproc_address_{"inproc://innerzmq"};
  void *context_{nullptr};
};

#endif /* WORKSPACE_MARKET_INFRA_ZMQBASE_H_ */
