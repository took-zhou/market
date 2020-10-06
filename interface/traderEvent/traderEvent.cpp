/*
 * traderEvent.cpp
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */


#include "market/interface/traderEvent/traderEvent.h"

#include "common/extern/log/log.h"
#include "market/infra/define.h"


bool TraderEvent::init()
{
    regMsgFun();

    return true;
}

void TraderEvent::handle(MsgStruct& msg)
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

void TraderEvent::regMsgFun()
{
    msgFuncMap.clear();
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("QryInstrumentRsp",   [this](MsgStruct& msg){qryInstrumentRspHandle(msg);}));

    return;
}

void TraderEvent::qryInstrumentRspHandle(MsgStruct& msg)
{

    return;
}

