/*
 * ctpMarketApi.cpp
 *
 *      Author: Administrator
 */
#include "market/infra/recer/ctpRecer.h"
#include "market/domain/components/ctpMarketApi/ctpMarketApi.h"
#include <string>

CtpMarketBaseApi::CtpMarketBaseApi(){
    marketApi = CreateFtdcMdApi();
}

CThostFtdcMdApi* CtpMarketBaseApi::CreateFtdcMdApi(const char *pszFlowPath , const bool bIsUsingUdp, const bool bIsMulticast)
{
    return CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath, bIsUsingUdp, bIsMulticast);
}

const char* CtpMarketBaseApi::GetApiVersion()
{
    return CThostFtdcMdApi::GetApiVersion();
}

void CtpMarketBaseApi::Release()
{
    return;
}

void CtpMarketBaseApi::Init()
{
    return;
}

int CtpMarketBaseApi::Join()
{
    return 0;
}

const char* CtpMarketBaseApi::GetTradingDay()
{
    return marketApi->GetTradingDay();
}

void CtpMarketBaseApi::RegisterFront(char *pszFrontAddress)
{
    return;
}

void CtpMarketBaseApi::RegisterNameServer(char *pszNsAddress)
{
    return;
}

void CtpMarketBaseApi::RegisterFensUserInfo(CThostFtdcFensUserInfoField * pFensUserInfo)
{
    return;
}

void CtpMarketBaseApi::RegisterSpi(CThostFtdcMdSpi *pSpi)
{
    marketApi->RegisterSpi(pSpi);
    return;
}

int CtpMarketBaseApi::SubscribeMarketData(char *ppInstrumentID[], int nCount)
{
    return 0;
}

int CtpMarketBaseApi::UnSubscribeMarketData(char *ppInstrumentID[], int nCount)
{
    return 0;
}

int CtpMarketBaseApi::SubscribeForQuoteRsp(char *ppInstrumentID[], int nCount)
{
    return 0;
}

int CtpMarketBaseApi::UnSubscribeForQuoteRsp(char *ppInstrumentID[], int nCount)
{
    return 0;
}

int CtpMarketBaseApi::ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID)
{
    return 0;
}

int CtpMarketBaseApi::ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID)
{
    return 0;
}


bool CtpMarketApi::init()
{
//    std::string pszFlowPath = "./flow";
//    marketApi->CreateFtdcMdApi(pszFlowPath.c_str());
//
//    auto& marketSpi = MarketSpi::getInstance();
//    marketApi->RegisterSpi(&marketSpi);
//
//
//
//    std::string marketFront = "";
//    auto tmpMarketFront = const_cast<char*>(marketFront.c_str());
//    marketApi->RegisterFront(tmpMarketFront);

    return true;
}

void CtpMarketApi::runLogInAndLogOutAlg()
{
    while(1)
    {
        ;
    }
    return;
}
