/*
 * zmqBase.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/infra/base_zmq.h"
#include "common/self/utils.h"

BaseZmq::BaseZmq() {
  context = zmq_ctx_new();
  utils::GetLocalIp(local_ip);
}
