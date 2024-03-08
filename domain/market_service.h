/*
 * market_service.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_
#define WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_

#include "common/self/dci/role.h"
#include "market/domain/components/diagnostic.h"
#include "market/domain/components/instrument_info.h"
#include "market/domain/components/market_time_state.h"
#include "market/domain/components/publish_control.h"
#include "market/domain/components/publish_depth_market_data.h"
#include "market/domain/components/publish_market_state.h"
#include "market/domain/components/store_depth_market_data.h"
#include "market/domain/components/subscribe_manage.h"

enum MarketLoginState { kErrorState = 0, kLoginState = 1, kLogoutState = 2, kManualExit = 3, kLossConnection = 4 };

struct MarketService : MarketTimeState, LoadData, PublishControl, PublishData, PublishState, SubscribeManager, InstrumentInfo, Diagnostic {
  MarketService();
  MarketService(const MarketService &) = delete;
  MarketService &operator=(const MarketService &) = delete;
  static MarketService &GetInstance() {
    static MarketService instance;
    return instance;
  }

  IMPL_ROLE(MarketTimeState);
  IMPL_ROLE(LoadData);
  IMPL_ROLE(PublishControl);
  IMPL_ROLE(PublishData);
  IMPL_ROLE(PublishState);
  IMPL_ROLE(SubscribeManager);
  IMPL_ROLE(InstrumentInfo);
  IMPL_ROLE(Diagnostic);

  bool UpdateLoginState(MarketLoginState state);
  MarketLoginState GetLoginState();

 private:
  void FastBackTask();
  void RealTimeTask();
  bool HandleErrorState();
  bool HandleLoginState();
  bool HandleLogoutState();
  bool HandleLossConnection();
  bool FastBackLoginLogoutChange();
  bool RealTimeLoginLogoutChange();
  void InitDatabase();
  MarketLoginState login_state_ = kLogoutState;
  uint32_t try_login_heartbeat_ = 0;
  uint32_t try_login_count_ = 0;
};

#endif /* WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_ */
