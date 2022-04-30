#include <unistd.h>

#include "market/domain/marketService.h"
#include "market/domain/components/activeSafety.h"
#include "common/extern/log/log.h"
#include "market/infra/recerSender.h"
#include "common/self/utils.h"

#include "common/self/semaphorePart.h"
extern GlobalSem globalSem;

activeSafety::activeSafety()
{
    ;;
}

void activeSafety::checkSafety()
{
    time_t now = {0};
    struct tm *timenow = NULL;
    static int check_flag = false;

    while(1)
    {
        time(&now);
        timenow = localtime(&now);//获取当前时间

        // 固定在下午6点开始检测
        if (timenow->tm_hour == 18 && timenow->tm_min == 0 && check_flag == false)
        {
            req_alive();
            check_flag = true;
        }
        else if (timenow->tm_hour == 18 && timenow->tm_min > 0 && check_flag == true)
        {
            check_flag = false;
        }

        sleep(1);
    }
}

void activeSafety::req_alive()
{
    INFO_LOG("is going to check target is alive.");

    auto& marketSer = MarketService::getInstance();
    auto keyNameList = marketSer.ROLE(controlPara).getKeyNameList();
    for (auto iter = keyNameList.begin(); iter != keyNameList.end(); iter++)
    {
        market_strategy::message msg;
        auto active_safety = msg.mutable_active_req();

        market_strategy::ActiveSafetyReq_MessageType check_id = market_strategy::ActiveSafetyReq_MessageType_isrun;
        active_safety->set_safe_id(check_id);
        active_safety->set_process_random_id(*iter);

        std::string reqStr;
        msg.SerializeToString(&reqStr);
        auto& recerSender = RecerSender::getInstance();
        string topic = "market_strategy.ActiveSafetyReq." + *iter;
        recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), reqStr.c_str());

        std::string semName = "req_alive";
        globalSem.addOrderSem(semName);
        if (globalSem.waitSemBySemName(semName, 1) != 0)
        {
            req_alive_timeout(*iter);
        }
        globalSem.delOrderSem(semName);
        sleep(1);
    }

    marketSer.ROLE(controlPara).write_to_json();
}

void activeSafety::req_alive_timeout(const string keyname)
{
    auto& marketSer = MarketService::getInstance();
    std::map<string, publishControl>::iterator iter = marketSer.ROLE(controlPara).publishCtrlMap.find(keyname);
    if (iter != marketSer.ROLE(controlPara).publishCtrlMap.end())
    {
        INFO_LOG("%s req alive timeout, will not subscribe.", iter->first.c_str());
        marketSer.ROLE(controlPara).publishCtrlMap.erase(iter);
        marketSer.ROLE(controlPara).write_to_json();
    }
}