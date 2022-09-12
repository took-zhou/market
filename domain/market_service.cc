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
  auto HeartBeatDetectFunc = [&]() { ROLE(PublishData).HeartBeatDetect(); };
  std::thread(HeartBeatDetectFunc).detach();

  // 开启主动安全监测线程
  auto CheckSafetyFuc = [&]() { ROLE(ActiveSafety).CheckSafety(); };
  std::thread(CheckSafetyFuc).detach();

  auto loginStateRun = [&]() { ROLE(MarketTimeState).Update(); };
  std::thread(loginStateRun).detach();

  auto marketLogInOutFuc = [&]() {
    auto &recerSender = RecerSender::getInstance();
    while (1) {
      if (ROLE(MarketTimeState).get_time_state() == kLoginTime && login_state == kLogoutState) {
        if (recerSender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
          login_state = kLoginState;
        } else {
          login_state = kErrorState;
        }
      } else if (ROLE(MarketTimeState).get_time_state() == kLogoutTime && login_state != kLogoutState) {
        recerSender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
        login_state = kLogoutState;
      } else if (recerSender.ROLE(Sender).ROLE(ItpSender).LossConnection() && login_state != kLogoutState) {
        recerSender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin();
      }

      std::this_thread::sleep_for(1000ms);
    }
  };
  std::thread(marketLogInOutFuc).detach();
  INFO_LOG("marketLogInOutFuc prepare ok");
}