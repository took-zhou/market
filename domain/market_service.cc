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
#include "market/domain/components/fd_manage.h"

MarketService::MarketService() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto api_type = json_cfg.GetConfig("common", "ApiType");

  if (api_type == "ftp") {
    auto market_period_task = [&]() {
      uint32_t period_count = 0;
      while (1) {
        // market_period_task begin
        FastBackLoginLogoutChange();
        if (period_count % 10 == 0) {
          FdManage::GetInstance().OpenThingsUp();
        }
        // market_period_task end
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
      login_state = kLoginState;
    } else {
      login_state = kErrorState;
      try_login_heartbeat_ = 0;
      try_login_count_ = 0;
    }
  } else if (ROLE(MarketTimeState).GetTimeState() == kLoginTime && login_state == kErrorState) {
    if (try_login_heartbeat_++ % 600 == 599 && try_login_count_++ <= 3 && recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      login_state = kLoginState;
    }
  } else if (ROLE(MarketTimeState).GetTimeState() == kLogoutTime && login_state != kLogoutState) {
    recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
    login_state = kLogoutState;
  } else if (recer_sender.ROLE(Sender).ROLE(ItpSender).LossConnection() && login_state != kLogoutState) {
    HandleAccountExitException();
  }

  return 0;
}

bool MarketService::FastBackLoginLogoutChange() {
  auto &recer_sender = RecerSender::GetInstance();
  if (login_state == kLogoutState) {
    if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
      login_state = kLoginState;
    } else {
      login_state = kErrorState;
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