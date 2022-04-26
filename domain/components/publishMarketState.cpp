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
    char date_buff[10];
    get_trade_data(date_buff);
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
        market_state->set_date(date_buff);
        std::string tickStr;
        tick.SerializeToString(&tickStr);
        auto& recerSender = RecerSender::getInstance();
        string topic = "market_strategy.TickMarketState." + *iter;
        recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
        sleep(1);
    }
    INFO_LOG("Publish makret state.");
}

int publishState::is_leap_year(int y)
{
    if ((y%4 == 0 && y%100 != 0) || y%400 == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void publishState::get_trade_data(char * buff)
{
    int y,m,d,a[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    time_t now_time = time(NULL);
    //local time
    tm* local_time = localtime(&now_time);

    y = 1900+local_time->tm_year;
    m = 1 + local_time->tm_mon;
    d = local_time->tm_mday;

    if (is_leap_year(y)==1)
    {
        a[1] = 29;
    }

    if (20 <= local_time->tm_hour && local_time->tm_hour <= 23)
    {
        if (local_time->tm_wday == 5)
        {
            d += 3;
            while(d>a[m-1])
            {
                d -= a[m-1];
                m++;
                if(m>12)
                {
                    m=1;
                    y++;
                    if (is_leap_year(y)==1)
                    {
                        a[1] = 29;
                    }
                    else
                    {
                        a[1] = 28;
                    }
                }
            }
            sprintf(buff, "%04d%02d%02d", y, m, d);
        }
        else
        {
            d += 1;
            while(d>a[m-1])
            {
                d -= a[m-1];
                m++;
                if(m>12)
                {
                    m=1;
                    y++;
                    if (is_leap_year(y)==1)
                    {
                        a[1] = 29;
                    }
                    else
                    {
                        a[1] = 28;
                    }
                }
            }
            sprintf(buff, "%04d%02d%02d", y, m, d);
        }
    }
    else if (1 <= local_time->tm_hour && local_time->tm_hour <= 3 && local_time->tm_wday == 6)
    {
        d += 2;
        while(d>a[m-1])
        {
            d -= a[m-1];
            m++;
            if(m>12)
            {
                m=1;
                y++;
                if (is_leap_year(y)==1)
                {
                    a[1] = 29;
                }
                else
                {
                    a[1] = 28;
                }
            }
        }
        sprintf(buff, "%04d%02d%02d", y, m, d);
    }
    else
    {
        sprintf(buff, "%04d%02d%02d", y, m, d);
    }

    INFO_LOG("trade date: %s", buff);
}
