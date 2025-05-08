#include "market/infra/sender/btp_sender.h"
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/infra/recer/btp_recer.h"

btp::api::MarketApi *BtpSender::market_api;
BtpMarketSpi *BtpSender::market_spi;

BtpSender::BtpSender(void) {}

BtpSender::~BtpSender() {}

bool BtpSender::Init(void) {
  bool out = true;
  if (!is_init_) {
    auto &json_cfg = utils::JsonConfig::GetInstance();
    market_api = btp::api::MarketApi::CreateMarketApi(json_cfg.GetFileName().c_str());
    if (market_api == nullptr) {
      out = false;
      INFO_LOG("quote api init fail.");
    } else {
      out = true;
      market_spi = new BtpMarketSpi();
      market_api->RegisterSpi(market_spi);
      INFO_LOG("quote api init ok.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    is_init_ = true;
  }
  return out;
}

bool BtpSender::ReqUserLogin(void) {
  INFO_LOG("login time, is going to login.");
  bool ret = true;
  if (!Init()) {
    Release();
    ret = false;
  } else {
    market_api->QryInstrumentInfo();
    market_api->Login();
  }

  return ret;
}

bool BtpSender::ReqUserLogout() {
  INFO_LOG("logout time, is going to logout.");

  if (market_api != nullptr) {
    market_api->Logout();
    Release();
  }

  return true;
}

bool BtpSender::Release() {
  INFO_LOG("is going to release quote api.");

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

bool BtpSender::SubscribeMarketData(std::vector<utils::InstrumentID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to un subscription.");
    return result;
  }

  unsigned int count = 0;
  char **pp_instrument_id = new char *[name_vec.size()];

  for (auto &item : name_vec) {
    pp_instrument_id[count] = const_cast<char *>(item.ins.c_str());
    count++;
  }

  if (count > 0) {
    result = market_api->SubscribeMarketData(pp_instrument_id, count, request_id);
    if (result != 0) {
      ERROR_LOG("subscribe market data fail, error code[%d]", result);
    }
  }

  delete[] pp_instrument_id;

  return true;
}

bool BtpSender::UnSubscribeMarketData(std::vector<utils::InstrumentID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to un subscription.");
    return result;
  }

  unsigned int count = 0;
  char **pp_instrument_id = new char *[name_vec.size()];

  for (auto &item : name_vec) {
    pp_instrument_id[count] = const_cast<char *>(item.ins.c_str());
    count++;
  }

  if (count > 0) {
    result = market_api->UnSubscribeMarketData(pp_instrument_id, count, request_id);
    if (result == 0) {
      INFO_LOG("un subscription request ......send a success, total number: %d", count);
    } else {
      ERROR_LOG("un subscription fail, error code[%d]", result);
    }
  }

  delete[] pp_instrument_id;

  return true;
}

bool BtpSender::LossConnection() { return false; }