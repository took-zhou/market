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
#include "market/domain/components/publishMarketState.h"
#include "market/domain/components/controlPara.h"
#include "market/domain/components/activeSafety.h"

struct MarketService: Market
                    , loadData
                    , controlPara
                    , publishData
                    , publishState
                    , activeSafety
{
    MarketService()
    {
        // 开启发布线程
        std::map<std::string, publishControl>::iterator mapit = ROLE(controlPara).publishCtrlMap.begin();
        while (mapit  != ROLE(controlPara).publishCtrlMap.end() && mapit->second.directforward == false)
        {
            mapit->second.thread_uniqueness_cnt = 0;
            // 开启发布线程
            auto publishDataFuc = [&](const string name){
                ROLE(publishData).publishToStrategy(name);
            };
            std::thread(publishDataFuc, mapit->first).detach();
            mapit++;
        }

        // 开启主动安全监测线程
        auto checkSafetyFuc = [&](){
            ROLE(activeSafety).checkSafety();
        };
        std::thread(checkSafetyFuc).detach();

        // 开启周期发送市场状态线程
        auto publishStateFuc = [&](){
            ROLE(publishState).pushlish_cycle();
        };
        std::thread(publishStateFuc).detach();
    }
    MarketService(const MarketService&) = delete;
    MarketService& operator=(const MarketService&) = delete;
    static MarketService& getInstance()
    {
        static MarketService instance;
        return instance;
    }

    IMPL_ROLE(Market);
    IMPL_ROLE(loadData);
    IMPL_ROLE(controlPara);
    IMPL_ROLE(publishData);
    IMPL_ROLE(publishState);
    IMPL_ROLE(activeSafety);
    TimeoutTimerPool& getTimeoutTimerPool();
};



#endif /* WORKSPACE_MARKET_DOMAIN_MARKETSERVICE_H_ */
