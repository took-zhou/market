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
        insVec.push_back(insId);
    }

    for (int i = 0; i < reqInfo.keyword_size(); i++)
    {
        keywordVec.push_back(reqInfo.keyword(i));
    }

    auto& marketSer = MarketService::getInstance();
    marketSer.ROLE(publishData).buildInstrumentList(insVec);
    marketSer.ROLE(publishData).buildKeywordList(keywordVec);
    marketSer.ROLE(publishData).setInterval(float(std::atof(reqInfo.interval().c_str())));

    if (marketSer.ROLE(Market).ROLE(CtpMarketApi).marketApi != nullptr)
    {
        marketSer.ROLE(Market).ROLE(CtpMarketApi).marketApi->SubscribeMarketData(insVec);
        // 开启发布线程
        auto publishDataFuc = [&](){
            marketSer.ROLE(publishData).publishToStrategy();
        };

        std::thread(publishDataFuc).detach();
    }
    else
    {
        ERROR_LOG("marketApi is nullptr.");
    }
}

void StrategyEvent::TickStartStopIndicationHandle(MsgStruct& msg)
{
    market_strategy::message _indication;
    _indication.ParseFromString(msg.pbMsg);
    auto indication = _indication.tick_start_stop_indication();

    auto& marketSer = MarketService::getInstance();
    marketSer.ROLE(publishData).setStartStopIndication(indication.type());
}
