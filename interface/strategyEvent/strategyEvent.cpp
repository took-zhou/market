/*
 * strategyEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "common/extern/log/log.h"
#include "common/self/protobuf/market-strategy.pb.h"
#include "common/self/utils.h"

#include "market/interface/strategyEvent/strategyEvent.h"
#include "market/infra/define.h"
#include "market/domain/components/ctpMarketApi/ctpMarketApi.h"
#include "market/domain/marketService.h"

bool StrategyEvent::init()
{
    regMsgFun();

    return true;
}

void StrategyEvent::regMsgFun()
{
    int cnt = 0;
    msgFuncMap.clear();
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("TickSubscribeReq", [this](MsgStruct& msg){TickSubscribeReqHandle(msg);}));
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("TickStartStopIndication", [this](MsgStruct& msg){TickStartStopIndicationHandle(msg);}));

    for(auto iter : msgFuncMap)
    {
        INFO_LOG("msgFuncMap[%d] key is [%s]",cnt, iter.first.c_str());
        cnt++;
    }
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

void StrategyEvent::TickSubscribeReqHandle(MsgStruct& msg)
{
    vector<string> keywordVec;
    vector<utils::InstrumtntID> insVec;
    string mapkeyname = "";
    insVec.clear();
    keywordVec.clear();

    market_strategy::message _reqInfo;
    _reqInfo.ParseFromString(msg.pbMsg);
    auto reqInfo = _reqInfo.tick_sub_req();

    for (int i = 0; i < reqInfo.instrument_info_list_size(); i++)
    {
        utils::InstrumtntID insId;
        insId.ins = reqInfo.instrument_info_list(i).instrument_id();
        if (reqInfo.instrument_info_list(i).exchange_id() == market_strategy::InstrumentInfo_ExchangeId_DCE)
        {
            insId.exch = "DCE";
        }
        else if (reqInfo.instrument_info_list(i).exchange_id() == market_strategy::InstrumentInfo_ExchangeId_SHFE)
        {
            insId.exch = "SHFE";
        }
        else if (reqInfo.instrument_info_list(i).exchange_id() == market_strategy::InstrumentInfo_ExchangeId_CZCE)
        {
            insId.exch = "CZCE";
        }
        else if (reqInfo.instrument_info_list(i).exchange_id() == market_strategy::InstrumentInfo_ExchangeId_CFFEX)
        {
            insId.exch = "CFFEX";
        }
        else if (reqInfo.instrument_info_list(i).exchange_id() == market_strategy::InstrumentInfo_ExchangeId_INE)
        {
            insId.exch = "INE";
        }
        
        insVec.push_back(insId);
        mapkeyname = mapkeyname + insId.ins;
    }

    for (int i = 0; i < reqInfo.keyword_size(); i++)
    {
        keywordVec.push_back(reqInfo.keyword(i));
    }

    auto& marketSer = MarketService::getInstance();
    marketSer.ROLE(publishData).buildInstrumentList(mapkeyname, insVec);
    marketSer.ROLE(publishData).buildKeywordList(mapkeyname, keywordVec);

    if (reqInfo.interval() == "raw")
    {
        marketSer.ROLE(publishData).setDirectForwardingFlag(mapkeyname, true);
    }
    else
    {
        marketSer.ROLE(publishData).setInterval(mapkeyname, float(std::atof(reqInfo.interval().c_str())));
        marketSer.ROLE(publishData).setDirectForwardingFlag(mapkeyname, false);
        //  开启发布线程
        auto publishDataFuc = [&](const string name){
            marketSer.ROLE(publishData).publishToStrategy(name);
        };
        std::thread(publishDataFuc, mapkeyname).detach();
    }

    if (marketSer.ROLE(Market).ROLE(CtpMarketApi).marketApi != nullptr)
    {
        marketSer.ROLE(Market).ROLE(CtpMarketApi).marketApi->SubscribeMarketData(insVec);
    }
    else
    {
        WARNING_LOG("not during login time, wait login time to subscribe new instruments");
    }
}

void StrategyEvent::TickStartStopIndicationHandle(MsgStruct& msg)
{
    string mapkeyname = "";
    market_strategy::message _indication;
    _indication.ParseFromString(msg.pbMsg);
    auto indication = _indication.tick_start_stop_indication();

    for (int i = 0; i < indication.instrument_info_list_size(); i++)
    {
        mapkeyname = mapkeyname + indication.instrument_info_list(i).instrument_id();
    }

    auto& marketSer = MarketService::getInstance();
    marketSer.ROLE(publishData).setStartStopIndication(mapkeyname, indication.type());
}
