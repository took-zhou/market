/*
 * zmqBase.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/base_zmq.h"
#include <unistd.h>
#include <sstream>
#include <string>
#include <thread>
#include "common/extern/log/log.h"

#include "common/self/file_util.h"
#include "common/self/utils.h"

BaseZmq::BaseZmq() {
  context = zmq_ctx_new();
  receiver = zmq_socket(context, ZMQ_SUB);
  publisher = zmq_socket(context, ZMQ_PUB);

  string local_ip;
  utils::GetLocalIp(local_ip);
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

void BaseZmq::SubscribeTopic(const char *topic_str) { zmq_setsockopt(receiver, ZMQ_SUBSCRIBE, topic_str, strlen(topic_str)); }

void BaseZmq::UnSubscribeTopic(const char *topic_str) { zmq_setsockopt(receiver, ZMQ_UNSUBSCRIBE, topic_str, strlen(topic_str)); }

int BaseZmq::SendMsg(const std::string &msg) {
  int size = zmq_send(publisher, const_cast<char *>(msg.c_str()), msg.length(), 0);
  return size;
}

int BaseZmq::RecvMsg(std::string &msg) {
  int size = zmq_recv(receiver, &msg[0], msg.length() - 1, 0);
  return size;
}