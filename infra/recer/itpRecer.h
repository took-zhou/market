#ifndef WORKSPACE_MARKET_INFRA_ITPRECER_H_
#define WORKSPACE_MARKET_INFRA_ITPRECER_H_

#include "market/infra/recer/recerSpi.h"

struct ItpRecer {
 public:
  ItpRecer();
  ItpRecer(const ItpRecer &) = delete;
  ItpRecer &operator=(const ItpRecer &) = delete;
  static ItpRecer &getInstance() {
    static ItpRecer instance;
    return instance;
  }

  bool receMsg(utils::ItpMsg &msg);

 private:
  RecerSpi *recerSpi = nullptr;
};

#endif