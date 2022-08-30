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

int InnerZmq::pushTask(const char *head, const char *msg) {
  std::stringstream tmpStr;
  tmpStr << head << " " << msg;

  int size = zmq_send(sender, const_cast<char *>(tmpStr.str().c_str()), strlen(const_cast<char *>(tmpStr.str().c_str())), 0);
  return size;
}

char *InnerZmq::pullTask() {
  enum { cap = 256 };
  char buffer[cap];
  int size = zmq_recv(receiver, buffer, cap - 1, 0);
  if (size == -1) return NULL;
  buffer[size < cap ? size : cap - 1] = '\0';

  return strndup(buffer, sizeof(buffer) - 1);
}
