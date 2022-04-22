#include "market/domain/components/publishCloseTime.h"
#include "market/domain/marketService.h"
#include "common/extern/log/log.h"
#include "market/infra/recerSender.h"

#include <sys/time.h>
#include <unistd.h>

publishCloseTime::publishCloseTime()
{
    ;;
}

void publishCloseTime::publish(void)
{
    char timeHMS[100] = {0};
    while(1)
    {
        auto& marketSer = MarketService::getInstance();

        std::map<string, publishControl>::iterator iter;
        for (iter = marketSer.ROLE(controlPara).publishCtrlMap.begin(); iter != marketSer.ROLE(controlPara).publishCtrlMap.end(); iter++)
        {
            getLocalHMS(timeHMS);
            string tempHMS = timeHMS;

            for (int i = 0; i < iter->second.closeTimeList.size(); i++)
            {
                if (iter->second.closeTimeList[i].close_start <= tempHMS && tempHMS <= iter->second.closeTimeList[i].close_stop)
                {
                    market_strategy::message tick;
                    auto close_time = tick.mutable_close_time();

                    close_time->set_result("yes");

                    std::string timeStr;
                    tick.SerializeToString(&timeStr);
                    auto& recerSender = RecerSender::getInstance();
                    string topic = "market_strategy.CloseTimeState." + iter->first;
                    recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), timeStr.c_str());

                    if (iter->second.closeTimeList[i].send_flag == false)
                    {
                        marketSer.ROLE(controlPara).setCloseTimeSendFlag(iter->first, i, true);
                    }
                }
                else if (iter->second.closeTimeList[i].send_flag == true)
                {
                    market_strategy::message tick;
                    auto close_time = tick.mutable_close_time();

                    close_time->set_result("no");

                    std::string timeStr;
                    tick.SerializeToString(&timeStr);
                    auto& recerSender = RecerSender::getInstance();
                    string topic = "market_strategy.CloseTimeState." + iter->first;
                    recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), timeStr.c_str());

                    marketSer.ROLE(controlPara).setCloseTimeSendFlag(iter->first, i, false);
                }
            }
        }

        sleep(1);
    }
}

bool publishCloseTime::getLocalHMS(char *t_arr)
{
    time_t now_time = time(NULL);
    tm* local_time = localtime(&now_time);
    sprintf(t_arr, "%02d:%02d:%02d", local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    return 0;
}
