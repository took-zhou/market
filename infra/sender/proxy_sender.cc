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
  auto &zmq_base = BaseZmq::GetInstance();
  std::string outstring = msg.session_name + "." + msg.msg_name + " " + msg.pb_msg;
  return zmq_base.SendMsg(outstring);
}
