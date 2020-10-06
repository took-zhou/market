/*
 * strategyEvent.cpp
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */


#include "market/interface/strategyEvent/strategyEvent.h"
#include "common/extern/log/log.h"
#include "market/infra/define.h"

bool StrategyEvent::init()
{
    regMsgFun();

    return true;
}


void StrategyEvent::regMsgFun()
{
    msgFuncMap.clear();
}

void StrategyEvent::handle(MsgStruct& msg)
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
