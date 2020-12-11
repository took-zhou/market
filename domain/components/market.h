/*
 * market.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_DOMAIN_COMPONENTS_MARKET_H_
#define WORKSPACE_MARKET_DOMAIN_COMPONENTS_MARKET_H_

#include "market/domain/components/ctpMarketApi/ctpMarketApi.h"
#include "market/domain/components/ctpMarketApi/marketLoginState.h"
#include "common/self/dci/Role.h"

struct Market: CtpMarketApi
{
    bool init()
    {
        // ROLE(CtpMarketApi).init();
        return true;
    }

    IMPL_ROLE(CtpMarketApi);
};



#endif /* WORKSPACE_MARKET_DOMAIN_COMPONENTS_MARKET_H_ */
