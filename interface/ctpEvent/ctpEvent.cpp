/*
 * ctpEvent.cpp
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */

#include "market/interface/ctpEvent/ctpEvent.h"
#include "common/extern/log/log.h"
#include "market/infra/define.h"

bool CtpEvent::init()
{
    regMsgFun();

    return true;
}


void CtpEvent::regMsgFun()
{
    msgFuncMap.clear();
}

void CtpEvent::handle(MsgStruct& msg)
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
