/*
 * ctpSender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/ctp_sender.h"
#include <cstddef>
#include <string>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/global_sem.h"
#include "common/self/utils.h"
#include "market/infra/recer/ctp_recer.h"

CThostFtdcMdApi *CtpSender::market_api;
CtpMarketSpi *CtpSender::market_spi;

CtpSender::CtpSender(void) {}

CtpSender::~CtpSender(void) {}

bool CtpSender::Init(void) {
  bool out = true;
  if (!is_init_) {
    auto &json_cfg = utils::JsonConfig::GetInstance();
    auto users = json_cfg.GetConfig("market", "User");
    for (auto &user : users) {
      con_path_ = json_cfg.GetConfig("market", "ConPath").get<std::string>() + "/" + static_cast<std::string>(user) + "/";
      utils::CreatFolder(con_path_);
      market_api = CThostFtdcMdApi::CreateFtdcMdApi(con_path_.c_str(), true, true);
      INFO_LOG("ctp version: %s.", market_api->GetApiVersion());

      market_spi = new CtpMarketSpi();
      market_api->RegisterSpi(market_spi);

      auto frontaddr = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "FrontMdAddr").get<std::string>();

      market_api->RegisterFront(const_cast<char *>(frontaddr.c_str()));
      market_api->Init();

      auto &global_sem = GlobalSem::GetInstance();
      if (global_sem.WaitSemBySemName(SemName::kLoginLogout, 10)) {
        out = false;
        ERROR_LOG("market init fail.");
      } else {
        out = true;
        INFO_LOG("market init ok.");
      }
      break;
    }
    is_init_ = true;
  }

  return out;
}

bool CtpSender::Release() {
  INFO_LOG("Is going to release market_api.");
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

bool CtpSender::ReqUserLogin() {
  bool ret = true;
  if (!Init()) {
    ret = false;
    Release();
  } else {
    CThostFtdcReqUserLoginField req_user_login{{0}};
    auto &json_cfg = utils::JsonConfig::GetInstance();

    auto users = json_cfg.GetConfig("market", "User");
    for (auto &user : users) {
      std::string user_id = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "UserID");
      std::string broker_id = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "BrokerID");
      std::string pass_word = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "Password");
      strcpy(req_user_login.BrokerID, broker_id.c_str());
      strcpy(req_user_login.UserID, user_id.c_str());
      strcpy(req_user_login.Password, pass_word.c_str());

      int result = market_api->ReqUserLogin(&req_user_login, request_id_++);
      if (result != 0) {
        INFO_LOG("ReqUserLogin send result is [%d]", result);
      } else {
        auto &global_sem = GlobalSem::GetInstance();
        global_sem.WaitSemBySemName(SemName::kLoginLogout);
      }

      break;
    }
  }
  return ret;
}

bool CtpSender::ReqUserLogout() {
  bool ret = true;
  CThostFtdcUserLogoutField req_user_logout{{0}};
  auto &json_cfg = utils::JsonConfig::GetInstance();

  INFO_LOG("logout time, is going to logout.");
  auto users = json_cfg.GetConfig("market", "User");
  for (auto &user : users) {
    auto user_id = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "UserID").get<std::string>();
    auto broker_id = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "BrokerID").get<std::string>();
    strcpy(req_user_logout.UserID, user_id.c_str());
    strcpy(req_user_logout.BrokerID, broker_id.c_str());

    if (market_api != nullptr) {
      int result = market_api->ReqUserLogout(&req_user_logout, request_id_++);
      if (result != 0) {
        INFO_LOG("ReqUserLogout send result is [%d]", result);
      }
      auto &global_sem = GlobalSem::GetInstance();
      if (global_sem.WaitSemBySemName(SemName::kLoginLogout, 10) != 0) {
        market_spi->OnRspUserLogout();
      }
      Release();
    }

    break;
  }
  return ret;
}

bool CtpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = 0;
  int md_num = 0;

  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to Subscription.");
    return result;
  }

  char **pp_instrument_id = new char *[name_vec.size()];

  for (auto &neme : name_vec) {
    pp_instrument_id[md_num] = const_cast<char *>(neme.ins.c_str());
    md_num++;
  }

  if (md_num > 0) {
    result = market_api->SubscribeMarketData(pp_instrument_id, md_num);
    if (result == 0) {
      INFO_LOG("Subscription request ......Send a success, total number: %d", md_num);
    } else {
      INFO_LOG("Subscription request ......Failed to send, error serial number=[%d]", result);
    }
  } else {
    INFO_LOG("no instrument need to Subscription.");
  }

  delete[] pp_instrument_id;

  return result;
}

bool CtpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = 0;
  int md_num = 0;

  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  char **pp_instrument_id = new char *[name_vec.size()];

  for (auto &name : name_vec) {
    pp_instrument_id[md_num] = const_cast<char *>(name.ins.c_str());
    md_num++;
  }

  if (md_num > 0) {
    result = market_api->UnSubscribeMarketData(pp_instrument_id, md_num);
    if (result == 0) {
      INFO_LOG("UnSubscription request ......Send a success, total number: %d", md_num);
    } else {
      INFO_LOG("UnSubscription request ......Failed to send, error serial number=[%d]", result);
    }
  } else {
    INFO_LOG("no instrument need to UnSubscription.");
  }

  delete[] pp_instrument_id;

  return result;
};

bool CtpSender::LossConnection() { return (market_spi != nullptr && market_spi->GetFrontDisconnect()); }
