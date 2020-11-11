/*
 * traderEvent.cpp
 *
 *  Created on: 2020��8��30��
 *      Author: Administrator
 */


#include "market/interface/traderEvent/traderEvent.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/extern/log/log.h"
#include "market/infra/define.h"
#include "common/self/utils.h"

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
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("QryInstrumentRsq",   [this](MsgStruct& msg){QryInstrumentRsqHandle(msg);}));
    return;
}

void TraderEvent::QryInstrumentRsqHandle(MsgStruct& msg)
{
    market_trader::message rspMsg;
    rspMsg.ParseFromString(msg.pbMsg);
    utils::printProtoMsg(rspMsg);

    auto& rsp = rspMsg.qry_instrument_rsp();
    auto& dataList = rsp.instrument_data_list();
    WARNING_LOG("instrument_data_list size [%u]", dataList.size());
}

void TraderEvent::qryInstrumentRspHandle(MsgStruct& msg)
{

    return;
}

