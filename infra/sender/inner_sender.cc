/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/inner_sender.h"
#include <thread>
#include "common/extern/libzmq/include/zmq.h"
#include "common/extern/log/log.h"
#include "market/infra/base_zmq.h"

InnerSender::InnerSender() {
  pusher_ = zmq_socket(BaseZmq::GetInstance().GetContext(), ZMQ_PUSH);

  int result = zmq_connect(pusher_, BaseZmq::GetInstance().GetInprocAddress().c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (result != 0) {
    ERROR_LOG("publisher connect to %s failed", BaseZmq::GetInstance().GetInprocAddress().c_str());
  } else {
    INFO_LOG("publisher connect to %s ok", BaseZmq::GetInstance().GetInprocAddress().c_str());
  }
}

bool InnerSender::SendMsg(utils::ItpMsg &msg) {
  std::string outstring;
  outstring += msg.session_name;
  outstring += ".";
  outstring += msg.msg_name;
  outstring += " ";
  outstring += msg.pb_msg;
  int size = zmq_send(pusher_, const_cast<char *>(outstring.c_str()), outstring.length(), 0);
  return (size > 0);
}
