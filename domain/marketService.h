/*
 * marketService.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_
#define WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_

#include "market/domain/components/market.h"
#include "common/self/timer.h"
#include "common/self/dci/Role.h"
#include "market/domain/components/storeDepthMarketData.h"
#include "market/domain/components/publishDepthMarketData.h"
struct MarketService: Market
                    , loadData
                    , publishData
{
    MarketService(){};
    MarketService(const MarketService&) = delete;
    MarketService& operator=(const MarketService&) = delete;
    static MarketService& getInstance()
    {
        static MarketService instance;
        return instance;
    }

    IMPL_ROLE(Market);
    IMPL_ROLE(loadData);
    IMPL_ROLE(publishData);
    TimeoutTimerPool& getTimeoutTimerPool();
};



#endif /* WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_ */
