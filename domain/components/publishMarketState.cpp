#include "market/domain/components/publishMarketState.h"
#include "common/self/protobuf/market-strategy.pb.h"
#include "market/infra/recerSender.h"
#include "market/domain/marketService.h"
#include "market/domain/components/ctpMarketApi/marketTimeState.h"
#include "common/extern/log/log.h"

#include <unistd.h>

publishState::publishState()
{
    ;;
}

void publishState::publish(void)
{
    auto& marketSer = MarketService::getInstance();

    auto keyNameList = marketSer.ROLE(publishData).getKeyNameList();
    for (auto iter = keyNameList.begin(); iter != keyNameList.end(); iter++)
    {
        market_strategy::message tick;
        auto market_state = tick.mutable_market_state();

        market_strategy::TickMaketState_MarketState state = market_strategy::TickMaketState_MarketState_reserve;
        if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_logout)
        {
            state = market_strategy::TickMaketState_MarketState_day_close;
        }
        else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_logout)
        {
            state = market_strategy::TickMaketState_MarketState_night_close;
        }
        else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_login)
        {
            state = market_strategy::TickMaketState_MarketState_day_open;
        }
        else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_login)
        {
            state = market_strategy::TickMaketState_MarketState_night_open;
        }
        market_state->set_market_state(state);
        std::string tickStr;
        tick.SerializeToString(&tickStr);
        auto& recerSender = RecerSender::getInstance();
        string topic = "market_strategy.TickMaketState." + *iter;
        recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
        sleep(1);
    }
    INFO_LOG("Publish makret state.");
}
