#ifndef WORKSPACE_MARKET_INFRA_ITPRECER_H_
#define WORKSPACE_MARKET_INFRA_ITPRECER_H_

#include "common/self/utils.h"

struct InnerRecer {
 public:
  InnerRecer();
  ~InnerRecer();
  bool ReceMsg(utils::ItpMsg &msg);

 private:
  void *puller_{nullptr};
  const int rec_timeout_ = 1000;
  std::string inner_address_;
};

#endif