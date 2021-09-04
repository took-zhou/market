#include "market/domain/components/publishMarketState.h"
#include "common/self/protobuf/market-strategy.pb.h"
#include "market/infra/recerSender.h"
#include "market/domain/marketService.h"

#include <unistd.h>

publishState::publishState()
{
    ;;
}

void publishState::publish(void)
{
    while(1)
    {
        auto& marketSer = MarketService::getInstance();
        if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGOUT_STATE)
        {
            if (publish_count++ <= 10)
            {
                market_strategy::message tick;
                auto close_state = tick.mutable_market_close();
                market_strategy::TickMaketClose_CloseState state = market_strategy::TickMaketClose_CloseState_close;
                close_state->set_close_state(state);

                std::string tickStr;
                tick.SerializeToString(&tickStr);

                auto& recerSender = RecerSender::getInstance();
                recerSender.ROLE(Sender).ROLE(ProxySender).send("market_strategy.TickMaketClose", tickStr.c_str());
            }
        }
        else
        {
            publish_count = 0;
        }
        sleep(60);
    }
}
