/*
 * ctpRecer.h
 *
 *  Created on: 2020年8月29日
 *      Author: Administrator
 */

#ifndef WORKSPACE_TRADER_INFRA_CTPRECER_H_
#define WORKSPACE_TRADER_INFRA_CTPRECER_H_
#include "common/extern/ctp/inc/ThostFtdcMdApi.h"
struct CtpRecer;
//行情类
class MarketSpi : public CThostFtdcMdSpi
{

public:
    MarketSpi(){};
    MarketSpi(const MarketSpi&) = delete;
    MarketSpi& operator=(const MarketSpi&) = delete;
    static MarketSpi& getInstance()
    {
        static MarketSpi instance;
        return instance;
    }

public:

    ~MarketSpi() {}
     void OnFrontConnected();

     void OnFrontDisconnected(int nReason);

     void OnHeartBeatWarning(int nTimeLapse);

    ///登录请求响应
     void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///登出请求响应
     void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///错误应答
     void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///订阅行情应答
     void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///取消订阅行情应答
     void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///订阅询价应答
     void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///取消订阅询价应答
     void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///深度行情通知
     void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    ///询价通知
     void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

     bool init( CtpRecer* _uper){
         uper = _uper;
         return true;
     };
public:
     CtpRecer* uper{nullptr};
};

struct CtpRecer
{
    bool init();
    CThostFtdcMdSpi*  ctpMdSpi{nullptr};
};



#endif /* WORKSPACE_TRADER_INFRA_CTPRECER_H_ */
