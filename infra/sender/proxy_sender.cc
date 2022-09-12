/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/proxy_sender.h"
#include "common/extern/log/log.h"
#include "market/infra/base_zmq.h"

bool ProxySender::Send(utils::ItpMsg &msg) {
  auto &zmqBase = BaseZmq::getInstance();
  std::string outstring = msg.sessionName + "." + msg.msgName + " " + msg.pbMsg;
  return zmqBase.SendMsg(outstring);
}
