#include "market/infra/recer/itpRecer.h"
#include "common/self/fileUtil.h"
#include "market/infra/innerZmq.h"

ItpRecer::ItpRecer() {}

bool ItpRecer::receMsg(utils::ItpMsg &msg) {
  auto &innerZmqBase = InnerZmq::getInstance();

  return innerZmqBase.pullTask(msg);
}
