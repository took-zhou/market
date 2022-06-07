/*
 * zmqBase.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include <unistd.h>
#include <sstream>
#include <string>

#include "common/extern/libzmq/include/zhelpers.h"
#include "common/extern/log/log.h"
#include "common/self/basetype.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "market/infra/zmqBase.h"

using json = nlohmann::json;

constexpr U32 WAITTIME_FOR_ZMQ_INIT = 1;

bool ZmqBase::init() {
  context = zmq_ctx_new();
  receiver = zmq_socket(context, ZMQ_SUB);
  publisher = zmq_socket(context, ZMQ_PUB);

  string local_ip;
  utils::get_local_ip(local_ip);
  string sub_ipaddport = "tcp://" + local_ip + ":8100";
  int result = zmq_connect(receiver, sub_ipaddport.c_str());
  sleep(WAITTIME_FOR_ZMQ_INIT);
  INFO_LOG("zmq_connect receiver result = %d", result);
  if (result != 0) {
    ERROR_LOG("receiver connect to %s failed", sub_ipaddport.c_str());
    return false;
  }

  string pub_ipaddport = "tcp://" + local_ip + ":5556";
  result = zmq_connect(publisher, pub_ipaddport.c_str());
  INFO_LOG("zmq_connect publisher result = %d", result);
  if (result != 0) {
    ERROR_LOG("publisher connect to %s failed", pub_ipaddport.c_str());
    return false;
  }

  INFO_LOG("zmq init ok");
  return true;
}

void ZmqBase::SubscribeTopic(const char *topicStr) { zmq_setsockopt(receiver, ZMQ_SUBSCRIBE, topicStr, strlen(topicStr)); }

void ZmqBase::unSubscribeTopic(const char *topicStr) { zmq_setsockopt(receiver, ZMQ_UNSUBSCRIBE, topicStr, strlen(topicStr)); }

int ZmqBase::PublishMsg(const char *head, const char *msg) {
  std::stringstream tmpStr;
  tmpStr << head << " " << msg;
  int ret2 = s_send(publisher, const_cast<char *>(tmpStr.str().c_str()));
  return ret2;
}
