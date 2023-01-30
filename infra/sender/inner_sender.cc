/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/inner_sender.h"
#include <thread>
#include "common/extern/log/log.h"
#include "market/infra/inner_zmq.h"

InnerSender::InnerSender() {
  pusher_ = zmq_socket(InnerZmq::GetInstance().context, ZMQ_PUSH);

  int result = zmq_connect(pusher_, InnerZmq::GetInstance().inproc_address.c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (result != 0) {
    ERROR_LOG("publisher connect to %s failed", InnerZmq::GetInstance().inproc_address.c_str());
  } else {
    INFO_LOG("publisher connect to %s ok", InnerZmq::GetInstance().inproc_address.c_str());
  }
}

bool InnerSender::SendMsg(utils::ItpMsg &msg) {
  m_lock_.lock();
  std::string outstring;
  outstring += msg.session_name;
  outstring += ".";
  outstring += msg.msg_name;
  outstring += " ";
  outstring += msg.pb_msg;
  int size = zmq_send(pusher_, const_cast<char *>(outstring.c_str()), outstring.length(), 0);
  m_lock_.unlock();
  return (bool)(size > 0);
}
