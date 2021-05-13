/*
 * ctpEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "common/self/fileUtil.h"

#include "market/interface/ctpEvent/ctpEvent.h"
#include "market/infra/define.h"
#include "market/domain/marketService.h"
#include "common/self/semaphorePart.h"

#include <string>
#include <sstream>
#include <unistd.h>

extern GlobalSem globalSem;
constexpr U32 WAITTIME_FOR_CONTRACTS_UNSCRIBEED = 1;

bool CtpEvent::init()
{
    regMsgFun();

    auto& jsonCfg = utils::JsonConfig::getInstance();
    reqInstrumentFrom = jsonCfg.getConfig("market","SubscribeMarketDataFrom").get<std::string>();
    return true;
}

void CtpEvent::regMsgFun()
{
    int cnt = 0;
    msgFuncMap.clear();
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("OnRtnDepthMarketData", [this](MsgStruct& msg){DeepMarktDataHandle(msg);}));
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("LoginInfo", [this](MsgStruct& msg){LoginInfoHandle(msg);}));
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("LogoutInfo", [this](MsgStruct& msg){LogoutInfoHandle(msg);}));

    for(auto iter : msgFuncMap)
    {
        INFO_LOG("msgFuncMap[%d] key is [%s]",cnt, iter.first.c_str());
        cnt++;
    }
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

void CtpEvent::DeepMarktDataHandle(MsgStruct& msg)
{
    auto deepdata = (CThostFtdcDepthMarketDataField*)msg.ctpMsg;
    auto& marketSer = MarketService::getInstance();

    if (reqInstrumentFrom == "trader")
    {
        marketSer.ROLE(loadData).LoadDepthMarketDataToCsv(deepdata);
    }
    else if (marketSer.ROLE(publishData).isDirectForwarding() == true)
    {
        marketSer.ROLE(publishData).directForwardDataToStrategy(deepdata);
    }
    else
    {
        marketSer.ROLE(publishData).insertDataToTickDataPool(deepdata);
    }

    delete deepdata;
}

void CtpEvent::LoginInfoHandle(MsgStruct& msg)
{
    auto ctpMsg = (CThostFtdcRspInfoField*)msg.ctpMsg;
    TThostFtdcErrorMsgType errormsg;
    utils::gbk2utf8(ctpMsg->ErrorMsg, errormsg, sizeof(errormsg));//报错返回信息

    INFO_LOG("ErrorCode=[%d], ErrorMsg=[%s]", ctpMsg->ErrorID, errormsg);

    if (ctpMsg->ErrorID != 0)
    {
        // 端登失败，客户端需进行错误处理
        ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", ctpMsg->ErrorID, errormsg);
        exit(-1);
    }
    else
    {
        auto& marketSer = MarketService::getInstance();

        if (reqInstrumentFrom == "local")
        {
            marketSer.ROLE(Market).ROLE(CtpMarketApi).reqInstrumentsFromLocal();
        }
        else if (reqInstrumentFrom == "trader")
        {
            marketSer.ROLE(Market).ROLE(CtpMarketApi).reqInstrumentsFromTrader();
        }
        else if (reqInstrumentFrom == "strategy")
        {
            marketSer.ROLE(Market).ROLE(CtpMarketApi).reqInstrumentsFromStrategy();
        }

        std::string semName = "market_login";
        globalSem.postSemBySemName(semName);
        INFO_LOG("post sem of [%s]", semName.c_str());
    }
}

void CtpEvent::LogoutInfoHandle(MsgStruct& msg)
{
    auto ctpMsg = (CThostFtdcRspInfoField*)msg.ctpMsg;
    TThostFtdcErrorMsgType errormsg;
    utils::gbk2utf8(ctpMsg->ErrorMsg, errormsg, sizeof(errormsg));//报错返回信息

    if (ctpMsg->ErrorID != 0)
    {
        // 端登失败，客户端需进行错误处理
        ERROR_LOG("Failed to login, errorcode=%d errormsg=%s", ctpMsg->ErrorID, errormsg);
        exit(-1);
    }
    else
    {
        UnSubscribeAllMarketData();
        sleep(WAITTIME_FOR_CONTRACTS_UNSCRIBEED);
        auto& marketSer = MarketService::getInstance();

        if (reqInstrumentFrom == "trader")
        {
            marketSer.ROLE(loadData).ClassifyContractFiles();
        }

        marketSer.ROLE(Market).release();
        delete (CThostFtdcRspInfoField*)ctpMsg;

        std::string semName = "market_logout";
        globalSem.postSemBySemName(semName);
        INFO_LOG("post sem of [%s]", semName.c_str());
    }
}

void CtpEvent::UnSubscribeAllMarketData(void)
{
    int instrumentCount = 0;
    auto& marketSer = MarketService::getInstance();
    vector<utils::InstrumtntID> ins_vec;
    auto iter = marketSer.ROLE(Market).marketApi->md_InstrumentIDs.instrumentIDs.begin();

    while (iter != marketSer.ROLE(Market).marketApi->md_InstrumentIDs.instrumentIDs.end())
    {
        ins_vec.push_back(*iter);
        if (ins_vec.size() >= 500)
        {
            marketSer.ROLE(Market).marketApi->UnSubscribeMarketData(ins_vec);
            ins_vec.clear();
        }
        iter++;
        instrumentCount++;
    }

    if (ins_vec.size() != 0)
    {
        marketSer.ROLE(Market).marketApi->UnSubscribeMarketData(ins_vec);
        ins_vec.clear();
    }

    INFO_LOG("The number of contracts being unsubscribe is: %d.", instrumentCount);
}
