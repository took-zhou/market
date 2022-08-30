/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/proxySender.h"
#include "common/extern/log/log.h"
#include "market/infra/zmqBase.h"


bool ProxySender::send(const char *head, const char *msg) {
  auto &zmqBase = ZmqBase::getInstance();
  return zmqBase.SendMsg(head, msg);
}
