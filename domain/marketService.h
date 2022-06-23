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
#include "market/domain/components/market.h"
#include "market/domain/components/publishDepthMarketData.h"
#include "market/domain/components/publishMarketState.h"
#include "market/domain/components/storeDepthMarketData.h"

struct MarketService : Market, loadData, controlPara, publishData, publishState, activeSafety {
  MarketService() {
    // 开启发布线程
    auto heartbeatDetect = [&]() { ROLE(publishData).heartbeatDetect(); };
    std::thread(heartbeatDetect).detach();

    // 开启主动安全监测线程
    auto checkSafetyFuc = [&]() { ROLE(activeSafety).checkSafety(); };
    std::thread(checkSafetyFuc).detach();
  }
  MarketService(const MarketService &) = delete;
  MarketService &operator=(const MarketService &) = delete;
  static MarketService &getInstance() {
    static MarketService instance;
    return instance;
  }

  IMPL_ROLE(Market);
  IMPL_ROLE(loadData);
  IMPL_ROLE(controlPara);
  IMPL_ROLE(publishData);
  IMPL_ROLE(publishState);
  IMPL_ROLE(activeSafety);
};

#endif /* WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_ */
