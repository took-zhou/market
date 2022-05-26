/*
 * ctpRecer.cpp
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#include "market/infra/recer/ctpRecer.h"
#include "market/infra/define.h"
#include "common/extern/libgo/libgo/libgo.h"
#include "common/extern/log/log.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"

extern GlobalSem globalSem;

extern co_chan<MsgStruct> ctpMsgChan;

void MarketSpi::OnFrontConnected()
{
    INFO_LOG("OnFrontConnected():is excuted...");
    // 在登出后系统会重新调用OnFrontConnected，这里简单判断并忽略第1次之后的所有调用。
    INFO_LOG("reConnect:%d.", reConnect);
    if (reConnect++ == 0)
    {
        std::string semName = "market_init";
        globalSem.postSemBySemName(semName);
        INFO_LOG("post sem of [%s]",semName.c_str());
    }
}

void MarketSpi::OnFrontDisconnected(int nReason)
{
    ERROR_LOG("OnFrontDisconnected, ErrorCode:%#x", nReason);
}

void MarketSpi::OnHeartBeatWarning(int nTimeLapse)
{
    ERROR_LOG("OnHeartBeatWarning  %d!",nTimeLapse);
}

void MarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    CThostFtdcRspInfoField *field = new CThostFtdcRspInfoField;
    memcpy(field, pRspInfo, sizeof(CThostFtdcRspInfoField));

    MsgStruct msgStruct;
    msgStruct.sessionName = "ctp";
    msgStruct.msgName = "LoginInfo";
    msgStruct.ctpMsg = field;
    ctpMsgChan << msgStruct;
}

void MarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    CThostFtdcRspInfoField *field = new CThostFtdcRspInfoField;
    memcpy(field, pRspInfo, sizeof(CThostFtdcRspInfoField));

    MsgStruct msgStruct;
    msgStruct.sessionName = "ctp";
    msgStruct.msgName = "LogoutInfo";
    msgStruct.ctpMsg = field;
    ctpMsgChan << msgStruct;
}

void MarketSpi::OnRspUserLogout(void)
{
    CThostFtdcRspInfoField *field = new CThostFtdcRspInfoField;
    field->ErrorID = 0;
    strcpy(field->ErrorMsg, "force logout");

    MsgStruct msgStruct;
    msgStruct.sessionName = "ctp";
    msgStruct.msgName = "LogoutInfo";
    msgStruct.ctpMsg = field;
    ctpMsgChan << msgStruct;
}

void MarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void MarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    #if 0
        INFO_LOG("<OnRspSubMarketData>");
        if (pSpecificInstrument)
        {
            INFO_LOG("\tInstrumentID = [%s]", pSpecificInstrument->InstrumentID);
        }
        if (pRspInfo)
        {
            INFO_LOG("\tErrorMsg = [%s]", pRspInfo->ErrorMsg);
            INFO_LOG("\tErrorID = [%d]", pRspInfo->ErrorID);
        }
        INFO_LOG("\tnRequestID = [%d]", nRequestID);
        INFO_LOG("\tbIsLast = [%d]", bIsLast);
        INFO_LOG("</OnRspSubMarketData>");
    #endif
}

void MarketSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    return;
}

void MarketSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void MarketSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

void MarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    CThostFtdcDepthMarketDataField *field = new CThostFtdcDepthMarketDataField;
    memcpy(field, pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));
    MsgStruct msgStruct;
    msgStruct.sessionName = "ctp";
    msgStruct.msgName = "OnRtnDepthMarketData";
    msgStruct.ctpMsg = field;
    ctpMsgChan << msgStruct;
}

void MarketSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    CThostFtdcForQuoteRspField *field = new CThostFtdcForQuoteRspField;
    memcpy(field, pForQuoteRsp, sizeof(CThostFtdcForQuoteRspField));

    MsgStruct msgStruct;
    msgStruct.sessionName = "ctp";
    msgStruct.msgName = "OnRtnForQuoteRsp";
    msgStruct.ctpMsg = field;
    ctpMsgChan << msgStruct;
}
