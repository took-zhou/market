/*
 * innerZmq.cpp
 *
 *  Created on: 2021年2月17日
 *      Author: Administrator
 */

#include "market/infra/inner_zmq.h"
#include "common/extern/log/log.h"

InnerZmq::InnerZmq() {
  context = zmq_ctx_new();
  receiver = zmq_socket(context, ZMQ_PULL);
  zmq_bind(receiver, inproc_address.c_str());

  sender = zmq_socket(context, ZMQ_PUSH);
  zmq_connect(sender, inproc_address.c_str());
}

int InnerZmq::PushTask(const utils::ItpMsg &msg) {
  std::string outstring = msg.session_name + "." + msg.msg_name + " " + msg.pb_msg;
  int size = zmq_send(sender, const_cast<char *>(outstring.c_str()), outstring.length(), 0);
  return size;
}

int InnerZmq::PullTask(utils::ItpMsg &msg) {
  bool out = true;

  std::string recv_string;
  recv_string.resize(256);

  int msgsize = zmq_recv(receiver, &recv_string[0], recv_string.length() - 1, 0);
  if (msgsize != -1) {
    int start_index = 0;
    int seg_index = 0;

    for (int i = 0; i < msgsize; i++) {
      if (recv_string[i] == '.') {
        if (seg_index == 0) {
          msg.session_name.resize(i - start_index);
          memcpy(&msg.session_name[0], &recv_string[start_index], (i - start_index));
        } else if (seg_index == 1) {
          msg.msg_name.resize(i - start_index);
          memcpy(&msg.msg_name[0], &recv_string[start_index], (i - start_index));
        }
        start_index = i + 1;
        seg_index++;
      } else if (recv_string[i] == ' ') {
        if (seg_index == 1) {
          i = i;
          msg.msg_name.resize(i - start_index);
          memcpy(&msg.msg_name[0], &recv_string[start_index], (i - start_index));
        }
        start_index = i + 1;
        break;
      }
    }

    msg.pb_msg.resize(msgsize - start_index);
    memcpy(&msg.pb_msg[0], &recv_string[start_index], (msgsize - start_index));
  } else {
    out = false;
  }

  return out;
}
