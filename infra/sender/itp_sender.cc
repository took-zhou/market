#include "market/infra/sender/itp_sender.h"
#include <memory>
#include "common/self/file_util.h"
#include "market/infra/sender/btp_sender.h"
#include "market/infra/sender/ctp_sender.h"
#include "market/infra/sender/ftp_sender.h"
#include "market/infra/sender/otp_sender.h"
#include "market/infra/sender/xtp_sender.h"

ItpSender::ItpSender() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto api_type = json_cfg.GetConfig("common", "ApiType");
  if (api_type == "ctp") {
    send_api_ = std::make_unique<CtpSender>();
  } else if (api_type == "xtp") {
    send_api_ = std::make_unique<XtpSender>();
  } else if (api_type == "btp") {
    send_api_ = std::make_unique<BtpSender>();
  } else if (api_type == "otp") {
    send_api_ = std::make_unique<OtpSender>();
  } else if (api_type == "ftp") {
    send_api_ = std::make_unique<FtpSender>();
  }
}

bool ItpSender::ReqUserLogin() { return send_api_->ReqUserLogin(); }
bool ItpSender::ReqUserLogout() { return send_api_->ReqUserLogout(); }
bool ItpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  return send_api_->SubscribeMarketData(name_vec, request_id);
}
bool ItpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  return send_api_->UnSubscribeMarketData(name_vec, request_id);
}
bool ItpSender::LossConnection() { return send_api_->LossConnection(); }
