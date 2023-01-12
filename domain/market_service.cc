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

MarketService::MarketService() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto api_type = json_cfg.GetConfig("common", "ApiType");
  if (api_type == "btp") {
    auto market_period_task = [&]() {
      // market_period_task begin

      // market_period_task end
      auto &recer_sender = RecerSender::GetInstance();
      while (1) {
        if (login_state == kLogoutState) {
          if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
            login_state = kLoginState;
          } else {
            login_state = kErrorState;
          }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    };
    std::thread(market_period_task).detach();
    INFO_LOG("market period task prepare ok");
  } else {
    auto market_period_task = [&]() {
      while (1) {
        // market_period_task begin
        ROLE(PublishData).HeartBeatDetect();
        ROLE(ActiveSafety).CheckSafety();
        ROLE(MarketTimeState).Update();
        ROLE(PublishState).PublishEvent();
        // market_period_task end

        auto &recer_sender = RecerSender::GetInstance();
        if (ROLE(MarketTimeState).GetTimeState() == kLoginTime && login_state == kLogoutState) {
          if (recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
            login_state = kLoginState;
          } else {
            login_state = kErrorState;
          }
        } else if (ROLE(MarketTimeState).GetTimeState() == kLogoutTime && login_state != kLogoutState) {
          recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
          login_state = kLogoutState;
        } else if (recer_sender.ROLE(Sender).ROLE(ItpSender).LossConnection() && login_state != kLogoutState) {
          HandleAccountExitException();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    };
    std::thread(market_period_task).detach();
    INFO_LOG("market period task prepare ok");
  }
}

bool MarketService::HandleAccountExitException() {
  bool ret = true;
  auto &recer_sender = RecerSender::GetInstance();

  ROLE(SubscribeManager).EraseAllSubscribed();
  ROLE(InstrumentInfo).EraseAllInstrumentInfo();
  recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin();
  return ret;
}