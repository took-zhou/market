/*
 * interactorEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "common/extern/log/log.h"
#include "common/self/protobuf/ctpview-market.pb.h"
#include "market/interface/ctpviewEvent/ctpviewEvent.h"
#include "market/infra/define.h"
#include "market/domain/marketService.h"

bool CtpviewEvent::init()
{
    regMsgFun();

    return true;
}
void CtpviewEvent::regMsgFun()
{
    int cnt = 0;
    msgFuncMap.clear();
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("LoginControl", [this](MsgStruct& msg){LoginControlHandle(msg);}));
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("CheckStrategyAlive", [this](MsgStruct& msg){CheckStrategyAliveHandle(msg);}));

    for(auto iter : msgFuncMap)
    {
        INFO_LOG("msgFuncMap[%d] key is [%s]",cnt, iter.first.c_str());
        cnt++;
    }
}

void CtpviewEvent::handle(MsgStruct& msg)
{
    auto iter = msgFuncMap.find(msg.msgName);
    if(iter != msgFuncMap.end())
    {
        iter->second(msg);
        return;
    }
    ERROR_LOG("can not find func for msgName [%s]!",msg.msgName.c_str());
    return;
}

void CtpviewEvent::LoginControlHandle(MsgStruct& msg)
{
    ctpview_market::message login_control;
    login_control.ParseFromString(msg.pbMsg);
    auto indication = login_control.login_control();

    int command = indication.command();
    auto& marketSer = MarketService::getInstance();

    marketSer.ROLE(Market).ROLE(MarketTimeState).set_time_state(command);
}

void CtpviewEvent::CheckStrategyAliveHandle(MsgStruct& msg)
{
    string command = "";
    ctpview_market::message check_alive;
    check_alive.ParseFromString(msg.pbMsg);
    auto indication = check_alive.check_alive();

    command = indication.check();
    auto& marketSer = MarketService::getInstance();

    if (command == "yes")
    {
        // 开启检测合约是否存活线程
        auto activeSafetyFunc = [&](){
            marketSer.ROLE(activeSafety).req_alive();
        };
        INFO_LOG("activeSafetyFunc prepare ok");
        std::thread(activeSafetyFunc).detach();
    }
}