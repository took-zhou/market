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
  std::string outstring = msg.sessionName + "." + msg.msgName + " " + msg.pbMsg;
  int size = zmq_send(sender, const_cast<char *>(outstring.c_str()), outstring.length(), 0);
  return size;
}

int InnerZmq::PullTask(utils::ItpMsg &msg) {
  bool out = true;

  std::string recvString;
  recvString.resize(256);

  int msgsize = zmq_recv(receiver, &recvString[0], recvString.length() - 1, 0);
  if (msgsize != -1) {
    int startIndex = 0;
    int segIndex = 0;

    for (int i = 0; i < msgsize; i++) {
      if (recvString[i] == '.') {
        if (segIndex == 0) {
          msg.sessionName.resize(i - startIndex);
          memcpy(&msg.sessionName[0], &recvString[startIndex], (i - startIndex));
        } else if (segIndex == 1) {
          msg.msgName.resize(i - startIndex);
          memcpy(&msg.msgName[0], &recvString[startIndex], (i - startIndex));
        }
        startIndex = i + 1;
        segIndex++;
      } else if (recvString[i] == ' ') {
        if (segIndex == 1) {
          i = i;
          msg.msgName.resize(i - startIndex);
          memcpy(&msg.msgName[0], &recvString[startIndex], (i - startIndex));
        }
        startIndex = i + 1;
        break;
      }
    }

    msg.pbMsg.resize(msgsize - startIndex);
    memcpy(&msg.pbMsg[0], &recvString[startIndex], (msgsize - startIndex));
  } else {
    out = false;
  }

  return out;
}
