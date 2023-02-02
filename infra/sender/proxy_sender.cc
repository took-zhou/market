/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/proxy_sender.h"
#include "common/extern/log/log.h"
#include "market/infra/base_zmq.h"

ProxySender::ProxySender() {
  publisher_ = zmq_socket(BaseZmq::GetInstance().context, ZMQ_PUB);

  string pub_ipaddport = "tcp://" + BaseZmq::GetInstance().local_ip + ":5556";
  int result = zmq_connect(publisher_, pub_ipaddport.c_str());

  if (result != 0) {
    ERROR_LOG("publisher connect to %s failed", pub_ipaddport.c_str());
  } else {
    INFO_LOG("publisher connect to %s ok", pub_ipaddport.c_str());
  }
}

bool ProxySender::SendMsg(utils::ItpMsg &msg) {
  m_lock_.lock();
  std::string outstring;
  outstring += msg.session_name;
  outstring += ".";
  outstring += msg.msg_name;
  outstring += " ";
  outstring += msg.pb_msg;
  int size = zmq_send(publisher_, const_cast<char *>(outstring.c_str()), outstring.length(), 0);
  m_lock_.unlock();
  return (bool)(size > 0);
}
