#include "market/infra/sender/itpSender.h"
#include "common/self/fileUtil.h"
#include "market/infra/sender/ctpSender.h"
#include "market/infra/sender/xtpSender.h"

ItpSender::ItpSender() {
  auto &jsonCfg = utils::JsonConfig::getInstance();
  auto apiType = jsonCfg.getConfig("common", "ApiType");
  if (apiType == "ctp") {
    sendApi = new CtpSender();
  } else if (apiType == "xtp") {
    sendApi = new XtpSender();
  }
}

bool ItpSender::ReqUserLogin() { sendApi->ReqUserLogin(); }
bool ItpSender::ReqUserLogout() { sendApi->ReqUserLogout(); }
bool ItpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) { sendApi->SubscribeMarketData(nameVec); }
bool ItpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) { sendApi->UnSubscribeMarketData(nameVec); }
bool ItpSender::ReqInstrumentInfo(const utils::InstrumtntID &ins) { sendApi->ReqInstrumentInfo(ins); }
bool ItpSender::LossConnection() { sendApi->LossConnection(); }