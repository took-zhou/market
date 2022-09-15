/*
 * marketService.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/domain/market_service.h"
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-trader.pb.h"

MarketService::MarketService() {
  // 开启发布线程
  auto heart_beat_detect_func = [&]() { ROLE(PublishData).HeartBeatDetect(); };
  std::thread(heart_beat_detect_func).detach();

  // 开启主动安全监测线程
  auto check_safety_fuc = [&]() { ROLE(ActiveSafety).CheckSafety(); };
  std::thread(check_safety_fuc).detach();

  auto login_state_run = [&]() { ROLE(MarketTimeState).Update(); };
  std::thread(login_state_run).detach();

  auto market_log_in_out_fuc = [&]() {
    auto &recer_sender = RecerSender::GetInstance();
    while (1) {
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
        recer_sender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin();
      }

      std::this_thread::sleep_for(1000ms);
    }
  };
  std::thread(market_log_in_out_fuc).detach();
  INFO_LOG("marketLogInOutFuc prepare ok");
}