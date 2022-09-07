/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/proxySender.h"
#include "common/extern/log/log.h"
#include "market/infra/zmqBase.h"

bool ProxySender::send(utils::ItpMsg &msg) {
  auto &zmqBase = ZmqBase::getInstance();
  std::string outstring = msg.sessionName + "." + msg.msgName + " " + msg.pbMsg;
  return zmqBase.SendMsg(outstring);
}
