/*
 * zmqBase.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/infra/base_zmq.h"
#include "common/self/utils.h"

BaseZmq::BaseZmq() {
  context_ = zmq_ctx_new();
  utils::GetLocalIp(local_ip_);
}

std::string &BaseZmq::GetLocalIp() { return local_ip_; }

std::string &BaseZmq::GetInprocAddress() { return inproc_address_; }

void *BaseZmq::GetContext() { return context_; }