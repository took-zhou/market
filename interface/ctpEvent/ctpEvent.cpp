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

extern GlobalSem globalSem;

bool CtpEvent::init()
{
    regMsgFun();

    auto& jsonCfg = utils::JsonConfig::getInstance();
    reqInstrumentFrom = jsonCfg.getConfig("market","SubscribeMarketDataFrom").get<std::string>();
    return true;
}

void CtpEvent::regMsgFun()
{
    msgFuncMap.clear();
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("OnRtnDepthMarketData", [this](MsgStruct& msg){DeepMarktDataHandle(msg);}));
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("LoginInfo", [this](MsgStruct& msg){LoginInfoHandle(msg);}));
    msgFuncMap.insert(std::pair<std::string, std::function<void(MsgStruct& msg)>>("LogoutInfo", [this](MsgStruct& msg){LogoutInfoHandle(msg);}));
    int cnt = 0;
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
    marketSer.ROLE(loadData).LoadDepthMarketDataToCsv(deepdata);

    if (reqInstrumentFrom != "trader")
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
        auto& marketSer = MarketService::getInstance();
        marketSer.ROLE(loadData).ClassifyContractFiles();
        marketSer.ROLE(Market).marketApi->Release();
        delete marketSer.ROLE(Market).marketApi;
        marketSer.ROLE(Market).marketApi = nullptr;
        delete ctpMsg;

        std::string semName = "market_logout";
        globalSem.postSemBySemName(semName);
        INFO_LOG("post sem of [%s]", semName.c_str());
    }
}
