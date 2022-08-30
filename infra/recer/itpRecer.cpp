#include "market/infra/recer/itpRecer.h"
#include "common/self/fileUtil.h"
#include "market/infra/recer/ctpRecer.h"
#include "market/infra/recer/xtpRecer.h"

ItpRecer::ItpRecer() {
  auto &jsonCfg = utils::JsonConfig::getInstance();
  auto apiType = jsonCfg.getConfig("common", "ApiType");
  if (apiType == "ctp") {
    recerSpi = new CtpRecer();
  } else if (apiType == "xtp") {
    recerSpi = new XtpRecer();
  }
}

bool ItpRecer::receMsg(utils::ItpMsg &msg) { recerSpi->receMsg(msg); }
