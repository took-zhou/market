/*
 * innerZmq.cpp
 *
 *  Created on: 2021年2月17日
 *      Author: Administrator
 */

#include "market/infra/innerZmq.h"
#include "common/extern/log/log.h"

InnerZmq::InnerZmq() {
  context = zmq_ctx_new();
  receiver = zmq_socket(context, ZMQ_PULL);
  zmq_bind(receiver, inprocAddress.c_str());

  sender = zmq_socket(context, ZMQ_PUSH);
  zmq_connect(sender, inprocAddress.c_str());
}

int InnerZmq::pushTask(const utils::ItpMsg &msg) {
  std::string outstring = msg.sessionName + "." + msg.msgName + " " + msg.pbMsg;
  int size = zmq_send(sender, const_cast<char *>(outstring.c_str()), outstring.length(), 0);
  return size;
}

int InnerZmq::pullTask(utils::ItpMsg &msg) {
  bool out = true;

  std::string recvString;
  recvString.resize(256);

  int msgsize = zmq_recv(receiver, &recvString[0], recvString.length() - 1, 0);
  if (msgsize != -1) {
    int index = 0;
    int segIndex = 0;
    char temp[msgsize];

    for (int i = 0; i < msgsize; i++) {
      temp[index] = recvString[i];
      if (temp[index] == '.') {
        if (segIndex == 0) {
          temp[index] = '\0';
          msg.sessionName = temp;
        } else if (segIndex == 1) {
          temp[index] = '\0';
          msg.msgName = temp;
        }
        segIndex++;
        index = 0;
      } else if (temp[index] == ' ') {
        if (segIndex == 1) {
          temp[index] = '\0';
          msg.msgName = temp;
        }
        segIndex++;
        index = 0;
      } else {
        index++;
      }
    }
    msg.pbMsg.resize(index);
    memcpy(&msg.pbMsg[0], temp, index);
  } else {
    out = false;
  }

  return out;
}
