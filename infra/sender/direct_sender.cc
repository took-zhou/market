/*
 * directSender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/direct_sender.h"
#include "common/extern/libzmq/include/zmq.h"
#include "common/extern/log/log.h"
#include "market/infra/base_zmq.h"

DirectSender::DirectSender() {
  publisher_ = zmq_socket(BaseZmq::GetInstance().GetContext(), ZMQ_PUB);

  string pub_ipaddport = "tcp://" + BaseZmq::GetInstance().GetLocalIp() + ":5557";
  int result = zmq_bind(publisher_, pub_ipaddport.c_str());

  if (result != 0) {
    ERROR_LOG("publisher connect to %s failed", pub_ipaddport.c_str());
  } else {
    INFO_LOG("publisher connect to %s ok", pub_ipaddport.c_str());
  }
}

bool DirectSender::SendMsg(utils::ItpMsg &msg) {
  std::string outstring;
  outstring += msg.session_name;
  outstring += ".";
  outstring += msg.msg_name;
  outstring += " ";
  outstring += msg.pb_msg;
  int size = zmq_send(publisher_, const_cast<char *>(outstring.c_str()), outstring.length(), 0);
  return (size > 0);
}
