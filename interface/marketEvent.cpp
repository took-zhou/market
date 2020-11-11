/*
 * marketEvent.cpp
 *
 *  Created on: 2020��9��9��
 *      Author: Administrator
 */
#include "market/interface/marketEvent.h"
#include "common/extern/libgo/libgo/libgo.h"
#include "market/infra/define.h"
#include "market/infra/recerSender.h"
#include <thread>
#include <chrono>
#include "common/extern/log/log.h"
#include "market/domain/marketService.h"

extern co_chan<MsgStruct> ctpMsgChan;

void MarketEvent::regSessionFunc()
{
    sessionFuncMap.clear();
    sessionFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct msg)>>("market_trader",         [this](MsgStruct msg){ROLE(TraderEvent).handle(msg);}));
    sessionFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct msg)>>("market_strategy",       [this](MsgStruct msg){ROLE(StrategyEvent).handle(msg);}));
    sessionFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct msg)>>("market_market",         [this](MsgStruct msg){ROLE(SelfEvent).handle(msg);}));
    sessionFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct msg)>>("interactor_market",     [this](MsgStruct msg){ROLE(InteractEvent).handle(msg);}));
    sessionFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct msg)>>("ctp",                   [this](MsgStruct msg){ROLE(CtpEvent).handle(msg);}));
}

bool MarketEvent::init()
{
    regSessionFunc();
    ROLE(TraderEvent).init();
    ROLE(StrategyEvent).init();
    ROLE(SelfEvent).init();
    ROLE(InteractEvent).init();
    ROLE(CtpEvent).init();
    return true;
}

bool MarketEvent::run()
{
    INFO_LOG("go into run");
    co::Scheduler* sched = co::Scheduler::Create();
    auto& recerSender = RecerSender::getInstance();

    auto proxyRecRun = [&](){
        while(1)
        {
            INFO_LOG("proxyRecRun while");
            MsgStruct msg = recerSender.ROLE(Recer).ROLE(ProxyRecer).receMsg();
            INFO_LOG("handle new msg, session is[%s],msgName is[%s]",msg.sessionName.c_str(), msg.msgName.c_str());
            if(! msg.isValid())
            {
//                ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]",msg.sessionName.c_str(), msg.msgName.c_str());
                continue;
            }
            auto eventFunc = [this, msg]{
                if(sessionFuncMap.find(msg.sessionName) != sessionFuncMap.end())
                {
                    sessionFuncMap[msg.sessionName](msg);
                    return;
                }
                ERROR_LOG("can not find[%s] in sessionFuncMap",msg.sessionName.c_str());
            };
            std::thread(eventFunc).detach();
        }
    };
    INFO_LOG("proxyRecRun prepare ok");
    std::thread(proxyRecRun).detach();   // zmq 在libgo的携程里会跑死，单独拎出来一个线程。

    auto ctpRecRun = [&](){
        MsgStruct msg;
        while(1)
        {
            INFO_LOG("ctpRecRun while");
            ctpMsgChan >> msg;
            if(! msg.isValid())
            {
                ERROR_LOG(" invalid msg, session is [%s], msgName is [%s]",msg.sessionName.c_str(), msg.msgName.c_str());
                continue;
            }
            auto eventFunc = [this, msg]{
                if(sessionFuncMap.find(msg.sessionName) != sessionFuncMap.end())
                {
                    sessionFuncMap[msg.sessionName](msg);
                }
            };
            std::thread(eventFunc).detach();
        }
    };
    INFO_LOG("ctpRecRun prepare ok");
    std::thread(ctpRecRun).detach();

    auto ctpMarketLogInOutFuc = [&](){
        auto& marketSer = MarketService::getInstance();
        marketSer.ROLE(Market).runLogInAndLogOutAlg();
    };

//    go co_scheduler(sched) proxyRecRun;
//    go co_scheduler(sched) ctpRecRun;
    go co_scheduler(sched) ctpMarketLogInOutFuc;

    std::thread([sched]{ sched->Start(0,10); }).join();
    return true;
}

