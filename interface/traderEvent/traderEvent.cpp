/*
 * traderEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/traderEvent/traderEvent.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/extern/log/log.h"
#include "market/infra/define.h"
#include "common/self/utils.h"
#include "market/domain/marketService.h"

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
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("QryInstrumentRsq",   [this](MsgStruct& msg){QryInstrumentRspHandle(msg);}));
    int cnt = 0;
    for(auto iter : msgFuncMap)
    {
        INFO_LOG("msgFuncMap[%d] key is [%s]",cnt, iter.first.c_str());
        cnt++;
    }
    return;
}

void TraderEvent::QryInstrumentRspHandle(MsgStruct& msg)
{
    int ik = pthread_mutex_lock(&sm_mutex);
    static int instrumentCount = 0;
    static vector<utils::InstrumtntID> ins_vec;
    auto& marketServer = MarketService::getInstance();

    market_trader::message rspMsg;
    rspMsg.ParseFromString(msg.pbMsg);

    auto& rsp = rspMsg.qry_instrument_rsp();
    auto& dataList = rsp.instrument_data_list();

    for (int i = 0; i < dataList.size(); i++)
    {
        utils::InstrumtntID instrumtntID;
        instrumtntID.exch = dataList.at(i).exchangeid();
        instrumtntID.ins = dataList.at(i).instrumentid();

        auto& marketSer = MarketService::getInstance();
        marketSer.ROLE(loadData).insertInsExchPair(instrumtntID.ins, instrumtntID.exch);
        ins_vec.push_back(instrumtntID);
        instrumentCount++;
    }

    if (rsp.finish_flag() == true)
    {
        marketServer.ROLE(Market).marketApi->SubscribeMarketData(ins_vec);
        INFO_LOG("The number of contracts being traded is: %d.", instrumentCount);
        instrumentCount = 0;
        ins_vec.clear();
    }
    else if (ins_vec.size() >= 500)
    {
        marketServer.ROLE(Market).marketApi->SubscribeMarketData(ins_vec);
        ins_vec.clear();
    }
    ik = pthread_mutex_unlock(&sm_mutex);
}
