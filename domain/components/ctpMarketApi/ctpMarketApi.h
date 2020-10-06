/*
 * ctpMarketApi.h
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_DOMAIN_COMPONENTS_CTPMARKETAPI_CTPMARKETAPI_H_
#define WORKSPACE_MARKET_DOMAIN_COMPONENTS_CTPMARKETAPI_CTPMARKETAPI_H_

#include "common/extern/ctp/inc/ThostFtdcMdApi.h"

class CtpMarketBaseApi : public CThostFtdcMdApi
{
public:
    CThostFtdcMdApi*  marketApi;
    CtpMarketBaseApi();
    CThostFtdcMdApi *CreateFtdcMdApi(const char *pszFlowPath = "", const bool bIsUsingUdp=false, const bool bIsMulticast=false);

    const char *GetApiVersion();

    void Release();

    void Init();

    int Join();

    const char *GetTradingDay();

    void RegisterFront(char *pszFrontAddress);

    void RegisterNameServer(char *pszNsAddress);

    void RegisterFensUserInfo(CThostFtdcFensUserInfoField * pFensUserInfo);

    void RegisterSpi(CThostFtdcMdSpi *pSpi);

    int SubscribeMarketData(char *ppInstrumentID[], int nCount);

    int UnSubscribeMarketData(char *ppInstrumentID[], int nCount);

    int SubscribeForQuoteRsp(char *ppInstrumentID[], int nCount);

    int UnSubscribeForQuoteRsp(char *ppInstrumentID[], int nCount);

    int ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID);

    int ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID);
};

struct CtpMarketApi
{
    bool init();
    void runLogInAndLogOutAlg();
    CtpMarketBaseApi* marketApi;
};


#endif /* WORKSPACE_MARKET_DOMAIN_COMPONENTS_CTPMARKETAPI_CTPMARKETAPI_H_ */
