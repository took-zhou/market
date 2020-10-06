/*
 * ctpRecer.cpp
 *
 *  Created on: 2020年8月30日
 *      Author: Administrator
 */

#include "market/infra/recer/ctpRecer.h"
#include "market/infra/define.h"
#include "common/extern/libgo/libgo/libgo.h"

extern co_chan<MsgStruct> ctpMsgChan;

void MarketSpi::OnFrontConnected()
{

}
void MarketSpi::OnFrontDisconnected(int nReason)
{

}


void MarketSpi::OnHeartBeatWarning(int nTimeLapse)
{

}


///登录请求响应
void MarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    static CThostFtdcRspUserLoginField staticRspUserLogin;
}

///登出请求响应
void MarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///错误应答
void MarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///订阅行情应答
void MarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///取消订阅行情应答
void MarketSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///订阅询价应答
void MarketSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///取消订阅询价应答
void MarketSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

}

///深度行情通知
void MarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    static CThostFtdcDepthMarketDataField staticDepthMarketDataField;
    staticDepthMarketDataField = *pDepthMarketData;
    MsgStruct msgStruct;
    msgStruct.sessionName = "ctp";
    msgStruct.msgName = "OnRtnDepthMarketData";
    msgStruct.ctpMsg = &staticDepthMarketDataField;
    ctpMsgChan << msgStruct;
}

///询价通知
void MarketSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    static CThostFtdcForQuoteRspField staticForQuoteRspField;
    staticForQuoteRspField = *pForQuoteRsp;
    MsgStruct msgStruct;
    msgStruct.sessionName = "ctp";
    msgStruct.msgName = "OnRtnForQuoteRsp";
    msgStruct.ctpMsg = &staticForQuoteRspField;
    ctpMsgChan << msgStruct;
}

bool CtpRecer::init(){
    auto& marketSpi = MarketSpi::getInstance();
    ctpMdSpi = &marketSpi;
    marketSpi.init(this);
    return true;
};


