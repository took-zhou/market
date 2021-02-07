/*
 * ctpRecer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_TRADER_INFRA_CTPRECER_H_
#define WORKSPACE_TRADER_INFRA_CTPRECER_H_
#include "common/extern/ctp/inc/ThostFtdcMdApi.h"

struct CtpRecer;

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

     void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

     void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

     // market端ctp登出没有反馈，主动调用反馈接口
     void OnRspUserLogout(void) ;

     void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

     void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

     void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

     void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

     void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

     void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

     void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

     bool init( CtpRecer* _uper){
         uper = _uper;
         return true;
     };
public:
     CtpRecer* uper{nullptr};
     int reConnect = 0;
};

struct CtpRecer
{
    bool init();
    CThostFtdcMdSpi*  ctpMdSpi{nullptr};
};

#endif /* WORKSPACE_TRADER_INFRA_CTPRECER_H_ */
