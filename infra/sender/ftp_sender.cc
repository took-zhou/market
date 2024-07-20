#include "market/infra/sender/ftp_sender.h"
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/infra/recer/ftp_recer.h"

ftp::api::MarketApi *FtpSender::market_api;
FtpMarketSpi *FtpSender::market_spi;

FtpSender::FtpSender(void) { ; }

bool FtpSender::Init(void) {
  bool out = true;
  if (!is_init_) {
    auto &json_cfg = utils::JsonConfig::GetInstance();
    auto users = json_cfg.GetConfig("market", "User");

    market_api = ftp::api::MarketApi::CreateMarketApi(json_cfg.GetFileName().c_str());
    market_spi = new FtpMarketSpi();
    market_api->RegisterSpi(market_spi);

    INFO_LOG("quote_api init ok.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    is_init_ = true;
  }
  return out;
}

bool FtpSender::ReqUserLogin(void) {
  INFO_LOG("login time, is going to login.");
  bool ret = true;
  Init();
  market_api->QryInstrumentInfo();
  market_api->Login();
  return ret;
}

bool FtpSender::ReqUserLogout() {
  INFO_LOG("logout time, is going to logout.");

  if (market_api != nullptr) {
    market_api->Logout();
    Release();
  }

  return true;
}

bool FtpSender::Release() {
  INFO_LOG("Is going to release quote_api.");

  if (market_api != nullptr) {
    market_api->Release();
    market_api = nullptr;
  }

  // 释放UserSpi实例
  if (market_spi != nullptr) {
    delete market_spi;
    market_spi = nullptr;
  }
  is_init_ = false;

  return true;
}

bool FtpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  unsigned int count = 0;
  char **pp_instrument_id = new char *[name_vec.size()];

  for (auto &item : name_vec) {
    pp_instrument_id[count] = const_cast<char *>(item.ins.c_str());
    count++;
  }

  if (count >= 0) {
    result = market_api->SubscribeMarketData(pp_instrument_id, count, request_id);
    if (result != 0) {
      ERROR_LOG("SubscribeMarketData fail, error code[%d]", result);
    }
  }

  delete[] pp_instrument_id;

  return true;
}

bool FtpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  unsigned int count = 0;
  char **pp_instrument_id = new char *[name_vec.size()];

  for (auto &item : name_vec) {
    pp_instrument_id[count] = const_cast<char *>(item.ins.c_str());
    count++;
  }

  if (count >= 0) {
    result = market_api->UnSubscribeMarketData(pp_instrument_id, count, request_id);
    if (result == 0) {
      INFO_LOG("UnSubscription request ......Send a success, total number: %d", count);
    } else {
      ERROR_LOG("UnSubscription fail, error code[%d]", result);
    }
  }

  delete[] pp_instrument_id;

  return true;
}

bool FtpSender::LossConnection() { return false; }
