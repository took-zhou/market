/*
 * marketService.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/domain/market_service.h"
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/utils.h"
#include "market/domain/components/fd_manage.h"
#include "market/domain/components/market_time_state.h"
#include "market/infra/recer_sender.h"

MarketService::MarketService() { InitDatabase(); }

MarketService::~MarketService() {}

bool MarketService::Run() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto api_type = json_cfg.GetConfig("common", "ApiType");

  running_ = true;
  if (api_type == "ftp") {
    fast_back_thread_ = std::thread(&MarketService::FastBackTask, this);
    INFO_LOG("market fast back thread start");
  } else {
    real_time_thread_ = std::thread(&MarketService::RealTimeTask, this);
    INFO_LOG("market real time thread start");
  }

  return true;
}

bool MarketService::Stop() {
  UpdateLoginState(MarketLoginState::kManualExit);
  INFO_LOG("set login state to manual exit.");
  running_ = false;
  if (fast_back_thread_.joinable()) {
    fast_back_thread_.join();
    INFO_LOG("market fast back thread exit");
  }
  if (real_time_thread_.joinable()) {
    real_time_thread_.join();
    INFO_LOG("market real time thread exit");
  }
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).Release();

  return true;
}

void MarketService::FastBackTask() {
  uint32_t period_count = 0;
  while (running_) {
    // market_period_task begin
    FastBackLoginLogoutChange();
    if (period_count % 1 == 0) {
      FdManage::GetInstance().OpenThingsUp();
    }
    // market_period_task end
    std::this_thread::sleep_for(std::chrono::seconds(1));
    period_count++;
  }
}

void MarketService::RealTimeTask() {
  uint32_t period_count = 0;
  while (running_) {
    // market_period_task begin
    ROLE(PublishData).HeartBeatDetect();
    ROLE(MarketTimeState).Update();
    ROLE(PublishState).PublishEvent();
    RealTimeLoginLogoutChange();
    if (period_count % 10 == 9) {
      ROLE(Diagnostic).MonitorStatus();
    }
    if (period_count % 2 == 1) {
      FdManage::GetInstance().OpenThingsUp();
    }
    // market_period_task end
    std::this_thread::sleep_for(std::chrono::seconds(1));
    period_count++;
  }
}

bool MarketService::RealTimeLoginLogoutChange() {
  if (login_state_ == kLogoutState) {
    HandleLogoutState();
  } else if (login_state_ == kErrorState) {
    HandleErrorState();
  } else if (login_state_ == kLoginState) {
    HandleLoginState();
  } else if (login_state_ == kLossConnection) {
    HandleLossConnection();
  }

  return 0;
}

bool MarketService::HandleErrorState() {
  auto &recer_sender = RecerSender::GetInstance();
  if (ROLE(MarketTimeState).GetTimeState() == kLoginTime) {
    if (try_login_heartbeat_++ % 600 == 599 && try_login_count_++ <= 3 && recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      UpdateLoginState(kLoginState);
    }
  } else if (ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
    recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
    UpdateLoginState(kLogoutState);
  }
  return true;
}

bool MarketService::HandleLoginState() {
  auto &recer_sender = RecerSender::GetInstance();
  if (ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
    recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
    UpdateLoginState(kLogoutState);
  } else if (recer_sender.ROLE(Sender).ROLE(ItpSender).LossConnection()) {
    UpdateLoginState(kLossConnection);
  }
  return true;
}

bool MarketService::HandleLogoutState() {
  auto &recer_sender = RecerSender::GetInstance();
  if (ROLE(MarketTimeState).GetTimeState() == kLoginTime) {
    try_login_heartbeat_ = 0;
    try_login_count_ = 0;
    if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      UpdateLoginState(kLoginState);
    } else {
      UpdateLoginState(kErrorState);
    }
  }
  return true;
}

bool MarketService::HandleLossConnection() {
  auto &recer_sender = RecerSender::GetInstance();
  if (ROLE(MarketTimeState).GetTimeState() == kLoginTime) {
    if (try_login_count_++ <= 60) {
      ROLE(SubscribeManager).EraseAllSubscribed();
      ROLE(InstrumentInfo).EraseAllInstrumentInfo();
      if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
        UpdateLoginState(kLoginState);
      }
    }
  } else if (ROLE(MarketTimeState).GetTimeState() == kLogoutTime) {
    recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
    UpdateLoginState(kLogoutState);
  }
  return true;
}

bool MarketService::FastBackLoginLogoutChange() {
  auto &recer_sender = RecerSender::GetInstance();
  if (login_state_ == kLogoutState) {
    if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      UpdateLoginState(kLoginState);
    } else {
      UpdateLoginState(kErrorState);
    }
  }

  return 0;
}

void MarketService::InitDatabase() {
  char *error_msg = nullptr;
  const char *sql = "create table if not exists service_info(compile_time TEXT, login_state INT);";
  if (sqlite3_exec(FdManage::GetInstance().GetMarketConn(), sql, NULL, NULL, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }

  sql = "insert into service_info(compile_time, login_state) select '', 3 where not exists (select * from service_info);";
  if (sqlite3_exec(FdManage::GetInstance().GetMarketConn(), sql, NULL, NULL, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}

bool MarketService::UpdateLoginState(MarketLoginState state) {
  char *error_msg = nullptr;
  char sql[100] = {0};

  login_state_ = state;
  sprintf(sql, "update service_info set compile_time='%s', login_state=%d;", utils::GetCompileTime().c_str(), login_state_);
  if (sqlite3_exec(FdManage::GetInstance().GetMarketConn(), sql, NULL, NULL, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
  return 0;
}

MarketLoginState MarketService::GetLoginState() { return login_state_; }