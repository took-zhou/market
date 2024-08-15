/*
 * zmqBase.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/infra/base_zmq.h"
#include "common/extern/libzmq/include/zmq.h"
#include "common/extern/log/log.h"
#include "common/self/utils.h"

BaseZmq::BaseZmq() {
  context_ = zmq_ctx_new();
  utils::GetLocalIp(local_ip_);
}

BaseZmq::~BaseZmq() {
  zmq_ctx_destroy(context_);
  INFO_LOG("zmq ctx destory ok.");
}

std::string &BaseZmq::GetLocalIp() { return local_ip_; }

std::string &BaseZmq::GetInnerAddress() { return inner_address_; }

void *BaseZmq::GetContext() { return context_; }