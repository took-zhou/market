#include "market/infra/recer/inner_recer.h"
#include <thread>
#include "common/extern/libzmq/include/zmq.h"
#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "market/infra/base_zmq.h"

InnerRecer::InnerRecer() {
  puller_ = zmq_socket(BaseZmq::GetInstance().GetContext(), ZMQ_PULL);
  zmq_setsockopt(puller_, ZMQ_RCVTIMEO, &rec_timeout_, sizeof(rec_timeout_));

  inner_address_ = BaseZmq::GetInstance().GetInnerAddress();
  int result = zmq_bind(puller_, inner_address_.c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (result != 0) {
    ERROR_LOG("puller bind to %s failed", inner_address_.c_str());
  } else {
    INFO_LOG("puller bind to %s ok", inner_address_.c_str());
  }
}

InnerRecer::~InnerRecer() {
  zmq_close(puller_);
  INFO_LOG("puller unbind to %s ok", inner_address_.c_str());
}

bool InnerRecer::ReceMsg(utils::ItpMsg &msg) {
  std::string recv_string;
  recv_string.resize(256);

  int msgsize = zmq_recv(puller_, &recv_string[0], recv_string.length() - 1, 0);
  return utils::ReceMsg(msg, recv_string, msgsize);
}
