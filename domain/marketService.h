/*
 * marketService.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_
#define WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_

#include "common/self/dci/Role.h"
#include "market/domain/components/activeSafety.h"
#include "market/domain/components/controlPara.h"
#include "market/domain/components/marketTimeState.h"
#include "market/domain/components/publishDepthMarketData.h"
#include "market/domain/components/publishMarketState.h"
#include "market/domain/components/storeDepthMarketData.h"
#include "market/domain/components/subscribeManage.h"
#include "market/infra/recerSender.h"

enum MARKET_LOGIN_STATE { ERROR_STATE = 0, LOGIN_STATE = 1, LOGOUT_STATE = 2 };

struct MarketService : MarketTimeState, loadData, controlPara, publishData, publishState, activeSafety, subscribeManager {
  MarketService();
  MarketService(const MarketService &) = delete;
  MarketService &operator=(const MarketService &) = delete;
  static MarketService &getInstance() {
    static MarketService instance;
    return instance;
  }

  IMPL_ROLE(MarketTimeState);
  IMPL_ROLE(loadData);
  IMPL_ROLE(controlPara);
  IMPL_ROLE(publishData);
  IMPL_ROLE(publishState);
  IMPL_ROLE(activeSafety);
  IMPL_ROLE(subscribeManager);

  MARKET_LOGIN_STATE login_state = LOGOUT_STATE;
};

#endif /* WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_ */
