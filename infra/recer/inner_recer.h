#ifndef WORKSPACE_MARKET_INFRA_ITPRECER_H_
#define WORKSPACE_MARKET_INFRA_ITPRECER_H_

#include "common/self/utils.h"

struct InnerRecer {
 public:
  InnerRecer();
  bool ReceMsg(utils::ItpMsg &msg);

 private:
  void *puller_{nullptr};
};

#endif