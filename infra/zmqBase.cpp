/*
 * zmqBase.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>

#include "common/extern/log/log.h"
#include "common/self/basetype.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "market/infra/zmqBase.h"

ZmqBase::ZmqBase() {
  context = zmq_ctx_new();
  receiver = zmq_socket(context, ZMQ_SUB);
  publisher = zmq_socket(context, ZMQ_PUB);

  string local_ip;
  utils::get_local_ip(local_ip);
  string sub_ipaddport = "tcp://" + local_ip + ":8100";
  int result = zmq_connect(receiver, sub_ipaddport.c_str());
  std::this_thread::sleep_for(1000ms);
  INFO_LOG("zmq_connect receiver result = %d", result);
  if (result != 0) {
    ERROR_LOG("receiver connect to %s failed", sub_ipaddport.c_str());
  }

  string pub_ipaddport = "tcp://" + local_ip + ":5556";
  result = zmq_connect(publisher, pub_ipaddport.c_str());
  INFO_LOG("zmq_connect publisher result = %d", result);
  if (result != 0) {
    ERROR_LOG("publisher connect to %s failed", pub_ipaddport.c_str());
  }

  INFO_LOG("zmq init ok");
}

void ZmqBase::SubscribeTopic(const char *topicStr) { zmq_setsockopt(receiver, ZMQ_SUBSCRIBE, topicStr, strlen(topicStr)); }

void ZmqBase::unSubscribeTopic(const char *topicStr) { zmq_setsockopt(receiver, ZMQ_UNSUBSCRIBE, topicStr, strlen(topicStr)); }

int ZmqBase::SendMsg(const char *head, const char *msg) {
  std::stringstream tmpStr;
  tmpStr << head << " " << msg;

  int size = zmq_send(publisher, const_cast<char *>(tmpStr.str().c_str()), strlen(const_cast<char *>(tmpStr.str().c_str())), 0);
  return size;
}

char *ZmqBase::RecvMsg() {
  enum { cap = 256 };
  char buffer[cap];
  int size = zmq_recv(receiver, buffer, cap - 1, 0);
  if (size == -1) return NULL;
  buffer[size < cap ? size : cap - 1] = '\0';

  return strndup(buffer, sizeof(buffer) - 1);
}