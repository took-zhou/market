/*
 * marketService.h
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_
#define WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_

#include "market/domain/components/market.h"
#include "common/self/timer.h"
#include "common/self/dci/Role.h"

struct MarketService: Market
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
    TimeoutTimerPool& getTimeoutTimerPool();
};



#endif /* WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_ */
