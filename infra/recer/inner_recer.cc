#include "market/infra/recer/inner_recer.h"
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/infra/base_zmq.h"

InnerRecer::InnerRecer() {
  puller_ = zmq_socket(BaseZmq::GetInstance().GetContext(), ZMQ_PULL);
  int result = zmq_bind(puller_, BaseZmq::GetInstance().GetInprocAddress().c_str());
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (result != 0) {
    ERROR_LOG("puller_ bind to %s failed", BaseZmq::GetInstance().GetInprocAddress().c_str());
  } else {
    INFO_LOG("puller_ bind to %s ok", BaseZmq::GetInstance().GetInprocAddress().c_str());
  }
}

bool InnerRecer::ReceMsg(utils::ItpMsg &msg) {
  std::string recv_string;
  recv_string.resize(256);

  int msgsize = zmq_recv(puller_, &recv_string[0], recv_string.length() - 1, 0);
  return utils::ReceMsg(msg, recv_string, msgsize);
}
