#include "market/infra/sender/itp_sender.h"
#include "common/self/file_util.h"
#include "market/infra/sender/ctp_sender.h"
#include "market/infra/sender/xtp_sender.h"

ItpSender::ItpSender() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto api_type = json_cfg.GetConfig("common", "ApiType");
  if (api_type == "ctp") {
    send_api_ = new CtpSender();
  } else if (api_type == "xtp") {
    send_api_ = new XtpSender();
  }
}

bool ItpSender::ReqUserLogin() { return send_api_->ReqUserLogin(); }
bool ItpSender::ReqUserLogout() { return send_api_->ReqUserLogout(); }
bool ItpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec) { return send_api_->SubscribeMarketData(name_vec); }
bool ItpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec) {
  return send_api_->UnSubscribeMarketData(name_vec);
}
bool ItpSender::ReqInstrumentInfo(const utils::InstrumtntID &ins) { return send_api_->ReqInstrumentInfo(ins); }
bool ItpSender::LossConnection() { return send_api_->LossConnection(); }