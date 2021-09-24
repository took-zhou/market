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
    while(1)
    {
        auto& marketSer = MarketService::getInstance();
        if (publish_count++ < 3)
        {
            auto publistList = marketSer.ROLE(publishData).getPublishList();
            for (auto iter = publistList.begin(); iter != publistList.end(); iter++)
            {
                market_strategy::message tick;
                auto close_state = tick.mutable_market_close();
                for (auto iter2 = iter->begin(); iter2 != iter->end(); iter2++)
                {
                    auto ins_exch = close_state->add_instrument_info_list();
                    ins_exch->set_instrument_id(iter2->ins);
                    ins_exch->set_exchange_id(iter2->exch);
                }
                market_strategy::TickMaketClose_CloseState state = market_strategy::TickMaketClose_CloseState_noclose;
                if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_logout)
                {
                    state = market_strategy::TickMaketClose_CloseState_day_close;
                }
                else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_logout)
                {
                    state = market_strategy::TickMaketClose_CloseState_night_close;
                }
                close_state->set_close_state(state);
                std::string tickStr;
                tick.SerializeToString(&tickStr);
                auto& recerSender = RecerSender::getInstance();
                recerSender.ROLE(Sender).ROLE(ProxySender).send("market_strategy.TickMaketClose", tickStr.c_str());
                sleep(1);
            }
            INFO_LOG("Publish makret close state.");
        }
        else
        {
            publish_count = 0;
            break;
        }
        sleep(1);
    }
}
