/*
 * market.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_DOMAIN_COMPONENTS_MARKET_H_
#define WORKSPACE_MARKET_DOMAIN_COMPONENTS_MARKET_H_

#include <thread>

#include "market/domain/components/ctpMarketApi/ctpMarketApi.h"
#include "market/domain/components/ctpMarketApi/marketTimeState.h"
#include "market/domain/components/publishMarketState.h"
#include "common/self/dci/Role.h"
#include "common/extern/log/log.h"

struct Market: CtpMarketApi
             , MarketTimeState
             , publishState
{
    bool init()
    {
        // 启动登录登出状态转换线程
        auto loginStateRun = [&](){
            ROLE(MarketTimeState).update();
        };
        INFO_LOG("loginStateRun prepare ok");
        std::thread(loginStateRun).detach();

        auto ctpMarketLogInOutFuc = [&](){
            ROLE(CtpMarketApi).runLogInAndLogOutAlg();
        };

        INFO_LOG("ctpMarketLogInOutFuc prepare ok");
        std::thread(ctpMarketLogInOutFuc).detach();

        auto publishMarketStateFuc = [&](){
            ROLE(publishState).publish();
        };

        INFO_LOG("publishMarketStateFuc prepare ok");
        std::thread(publishMarketStateFuc).detach();

        return true;
    }

    IMPL_ROLE(CtpMarketApi);
    IMPL_ROLE(MarketTimeState);
    IMPL_ROLE(publishState);
};



#endif /* WORKSPACE_MARKET_DOMAIN_COMPONENTS_MARKET_H_ */
