#include "market/infra/sender/ytp_sender.h"
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/global_sem.h"
#include "common/self/utils.h"
#include "market/infra/recer/ytp_recer.h"

ytp::api::MarketApi *YtpSender::market_api;
YtpMarketSpi *YtpSender::market_spi;

YtpSender::YtpSender(void) {}

YtpSender::~YtpSender(void) {}

bool YtpSender::Init(void) {
  bool out = true;
  if (!is_init_) {
    auto &json_cfg = utils::JsonConfig::GetInstance();
    auto users = json_cfg.GetConfig("market", "User");
    market_api = ytp::api::MarketApi::CreateMarketApi(json_cfg.GetFileName().c_str());
    if (market_api == nullptr) {
      out = false;
      INFO_LOG("quote api init fail.");
    } else {
      out = true;
      market_spi = new YtpMarketSpi();
      market_api->RegisterSpi(market_spi);

      INFO_LOG("quote api init ok.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    is_init_ = true;
  }
  return out;
}

bool YtpSender::ReqUserLogin(void) {
  INFO_LOG("login time, is going to login.");
  auto &global_sem = GlobalSem::GetInstance();
  bool ret = true;
  if (!Init()) {
    Release();
    ret = false;
  } else {
    for (uint8_t wait_count = 0; wait_count < 10; wait_count++) {
      market_api->QryInstrumentInfo();
      INFO_LOG("update instrument info from market send ok, waiting market rsp.");
      if (!global_sem.WaitSemBySemName(SemName::kUpdateInstrumentInfo, 60)) {
        break;
      }
    }

    YtpLoginLogoutStruct login_struct;
    market_api->Login(login_struct);
    global_sem.WaitSemBySemName(SemName::kLoginLogout);
  }

  return ret;
}

bool YtpSender::ReqUserLogout() {
  INFO_LOG("logout time, is going to logout.");

  if (market_api != nullptr) {
    YtpLoginLogoutStruct logout_struct;
    market_api->Logout(logout_struct);
    Release();
  }

  return true;
}

bool YtpSender::Release() {
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

bool YtpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
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

bool YtpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
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

bool YtpSender::LossConnection() { return market_spi->GetFrontDisconnected(); }
