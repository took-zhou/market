/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/proxy_sender.h"
#include "common/extern/libzmq/include/zmq.h"
#include "common/extern/log/log.h"
#include "market/infra/base_zmq.h"

ProxySender::ProxySender() {
  publisher_ = zmq_socket(BaseZmq::GetInstance().GetContext(), ZMQ_PUB);

  pub_ipaddport_ = "tcp://" + BaseZmq::GetInstance().GetLocalIp() + ":5556";
  int result = zmq_connect(publisher_, pub_ipaddport_.c_str());

  if (result != 0) {
    ERROR_LOG("publisher connect to %s failed", pub_ipaddport_.c_str());
  } else {
    INFO_LOG("publisher connect to %s ok", pub_ipaddport_.c_str());
  }
}

ProxySender::~ProxySender() {
  zmq_close(publisher_);
  INFO_LOG("publisher disconnect to %s ok.", pub_ipaddport_.c_str());
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
  return (size > 0);
}
