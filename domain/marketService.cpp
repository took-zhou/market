/*
 * marketService.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/domain/marketService.h"
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-trader.pb.h"
MarketService::MarketService() {
  // 开启发布线程
  auto heartbeatDetect = [&]() { ROLE(publishData).heartbeatDetect(); };
  std::thread(heartbeatDetect).detach();

  // 开启主动安全监测线程
  auto checkSafetyFuc = [&]() { ROLE(activeSafety).checkSafety(); };
  std::thread(checkSafetyFuc).detach();

  auto loginStateRun = [&]() { ROLE(MarketTimeState).update(); };
  std::thread(loginStateRun).detach();

  auto marketLogInOutFuc = [&]() {
    auto &recerSender = RecerSender::getInstance();
    while (1) {
      if (ROLE(MarketTimeState).output.status == LOGIN_TIME && login_state == LOGOUT_STATE) {
        if (recerSender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin()) {
          login_state = LOGIN_STATE;
        } else {
          login_state = ERROR_STATE;
        }
      } else if (ROLE(MarketTimeState).output.status == LOGOUT_TIME && login_state != LOGOUT_STATE) {
        recerSender.ROLE(Sender).ROLE(ItpSender).ReqUserLogout();
        login_state = LOGOUT_STATE;
      } else if (recerSender.ROLE(Sender).ROLE(ItpSender).LossConnection() && login_state != LOGOUT_STATE) {
        recerSender.ROLE(Sender).ROLE(ItpSender).ReqUserLogin();
      }

      std::this_thread::sleep_for(1000ms);
    }
  };
  std::thread(marketLogInOutFuc).detach();
  INFO_LOG("marketLogInOutFuc prepare ok");
}