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

MarketService::MarketService() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto api_type = json_cfg.GetConfig("common", "ApiType");

  InitDatabase();
  if (api_type == "ftp") {
    auto market_period_task = [&]() {
      uint32_t period_count = 0;
      while (1) {
        // market_period_task begin
        FastBackLoginLogoutChange();
        if (period_count % 1 == 0) {
          FdManage::GetInstance().OpenThingsUp();
        }
        // market_period_task end
        std::this_thread::sleep_for(std::chrono::seconds(1));
        period_count++;
      }
    };
    std::thread(market_period_task).detach();
    INFO_LOG("market period task prepare ok");
  } else {
    auto market_period_task = [&]() {
      uint32_t period_count = 0;
      while (1) {
        // market_period_task begin
        ROLE(PublishData).HeartBeatDetect();
        ROLE(MarketTimeState).Update();
        ROLE(PublishState).PublishEvent();
        RealTimeLoginLogoutChange();
        if (period_count % 10 == 0) {
          FdManage::GetInstance().OpenThingsUp();
        }
        // market_period_task end
        std::this_thread::sleep_for(std::chrono::seconds(1));
        period_count++;
      }
    };
    std::thread(market_period_task).detach();
    INFO_LOG("market period task prepare ok");
  }
}

bool MarketService::RealTimeLoginLogoutChange() {
  auto &recer_sender = RecerSender::GetInstance();
  if (ROLE(MarketTimeState).GetTimeState() == kLoginTime && login_state == kLogoutState) {
    if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      UpdateLoginState(kLoginState);
    } else {
      UpdateLoginState(kErrorState);
      try_login_heartbeat_ = 0;
      try_login_count_ = 0;
    }
  } else if (ROLE(MarketTimeState).GetTimeState() == kLoginTime && login_state == kErrorState) {
    if (try_login_heartbeat_++ % 600 == 599 && try_login_count_++ <= 3 && recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      UpdateLoginState(kLoginState);
    }
  } else if (ROLE(MarketTimeState).GetTimeState() == kLogoutTime && login_state != kLogoutState) {
    recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
    UpdateLoginState(kLogoutState);
  } else if (recer_sender.ROLE(Sender).ROLE(ItpSender).LossConnection() && login_state != kLogoutState) {
    HandleAccountExitException();
  }

  return 0;
}

bool MarketService::FastBackLoginLogoutChange() {
  auto &recer_sender = RecerSender::GetInstance();
  if (login_state == kLogoutState) {
    if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      UpdateLoginState(kLoginState);
    } else {
      UpdateLoginState(kErrorState);
    }
  }

  return 0;
}

bool MarketService::HandleAccountExitException() {
  bool ret = true;
  auto &recer_sender = RecerSender::GetInstance();

  ROLE(SubscribeManager).EraseAllSubscribed();
  ROLE(InstrumentInfo).EraseAllInstrumentInfo();
  recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin();
  return ret;
}

void MarketService::InitDatabase() {
  if (init_database_flag_ == false) {
    char *error_msg = nullptr;
    const char *sql = "drop table if exists service_info;";
    if (sqlite3_exec(FdManage::GetInstance().market_conn, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
      ERROR_LOG("Sql error %s.", error_msg);
      sqlite3_free(error_msg);
      sqlite3_close(FdManage::GetInstance().market_conn);
    }

    sql = "create table if not exists service_info(compile_time TEXT, login_state INT);";
    if (sqlite3_exec(FdManage::GetInstance().market_conn, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
      ERROR_LOG("Sql error %s.", error_msg);
      sqlite3_free(error_msg);
      sqlite3_close(FdManage::GetInstance().market_conn);
    }

    sql = "insert into service_info(compile_time, login_state) select '', 2 where not exists (select * from service_info);";
    if (sqlite3_exec(FdManage::GetInstance().market_conn, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
      ERROR_LOG("Sql error %s.", error_msg);
      sqlite3_free(error_msg);
      sqlite3_close(FdManage::GetInstance().market_conn);
    }
    init_database_flag_ = true;
  }
}

bool MarketService::UpdateLoginState(MarketLoginState state) {
  char *error_msg = nullptr;
  char sql[100] = {0};

  login_state = state;
  sprintf(sql, "update service_info set compile_time='%s', login_state=%d;", utils::GetCompileTime().c_str(), login_state);
  if (sqlite3_exec(FdManage::GetInstance().market_conn, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
    ERROR_LOG("Sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().market_conn);
  }
  return 0;
}