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

void publishState::publish_event(void)
{
    auto& marketSer = MarketService::getInstance();

    auto keyNameList = marketSer.ROLE(controlPara).getKeyNameList();
    for (auto iter = keyNameList.begin(); iter != keyNameList.end(); iter++)
    {
        market_strategy::message tick;
        auto market_state = tick.mutable_market_state();

        market_strategy::TickMarketState_MarketState state = market_strategy::TickMarketState_MarketState_reserve;
        if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_logout)
        {
            state = market_strategy::TickMarketState_MarketState_day_close;
        }
        else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_logout)
        {
            state = market_strategy::TickMarketState_MarketState_night_close;
        }
        else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_login)
        {
            state = market_strategy::TickMarketState_MarketState_day_open;
        }
        else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_login)
        {
            state = market_strategy::TickMarketState_MarketState_night_open;
        }

        market_state->set_market_state(state);
        std::string tickStr;
        tick.SerializeToString(&tickStr);
        auto& recerSender = RecerSender::getInstance();
        string topic = "market_strategy.TickMarketState." + *iter;
        recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
        sleep(1);
    }
    INFO_LOG("Publish makret state.");
}

void publishState::pushlish_cycle(void)
{
    time_t now = {0};
    struct tm *timenow = NULL;
    static int day_closing_published_flag = false;

    while(1)
    {
        time(&now);
        timenow = localtime(&now);//获取当前时间

        // 固定在14:55发生市场要收盘信号
        if (timenow->tm_hour == 14 && timenow->tm_min == 55 && day_closing_published_flag == false)
        {
            publish_day_closing();
            day_closing_published_flag = true;
        }

        if (timenow->tm_hour == 17 && timenow->tm_min == 0)
        {
            day_closing_published_flag = false;
        }
        sleep(1);
    }
}

void publishState::publish_day_closing(void)
{
    auto& marketSer = MarketService::getInstance();

    auto keyNameList = marketSer.ROLE(controlPara).getKeyNameList();
    for (auto iter = keyNameList.begin(); iter != keyNameList.end(); iter++)
    {
        market_strategy::message tick;
        auto market_state = tick.mutable_market_state();

        market_strategy::TickMarketState_MarketState state = market_strategy::TickMarketState_MarketState_day_closing;
        market_state->set_market_state(state);
        std::string tickStr;
        tick.SerializeToString(&tickStr);
        auto& recerSender = RecerSender::getInstance();
        string topic = "market_strategy.TickMarketState." + *iter;
        recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
        sleep(1);
    }
    INFO_LOG("Publish makret state.");
}
