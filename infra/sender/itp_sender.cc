#include "market/infra/sender/itp_sender.h"
#include "common/self/file_util.h"
#include "market/infra/sender/ctp_sender.h"
#include "market/infra/sender/xtp_sender.h"

ItpSender::ItpSender() {
  auto &jsonCfg = utils::JsonConfig::getInstance();
  auto apiType = jsonCfg.get_config("common", "ApiType");
  if (apiType == "ctp") {
    send_api = new CtpSender();
  } else if (apiType == "xtp") {
    send_api = new XtpSender();
  }
}

bool ItpSender::ReqUserLogin() { send_api->ReqUserLogin(); }
bool ItpSender::ReqUserLogout() { send_api->ReqUserLogout(); }
bool ItpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) { send_api->SubscribeMarketData(nameVec); }
bool ItpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) { send_api->UnSubscribeMarketData(nameVec); }
bool ItpSender::ReqInstrumentInfo(const utils::InstrumtntID &ins) { send_api->ReqInstrumentInfo(ins); }
bool ItpSender::LossConnection() { send_api->LossConnection(); }