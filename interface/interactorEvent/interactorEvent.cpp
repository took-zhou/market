/*
 * interactorEvent.cpp
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */


#include "market/interface/interactorEvent/interactorEvent.h"
#include "common/extern/log/log.h"
#include "market/infra/define.h"

bool InteractEvent::init()
{
    regMsgFun();

    return true;
}

void InteractEvent::regMsgFun()
{
    msgFuncMap.clear();
}

void InteractEvent::handle(MsgStruct& msg)
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
