/*
 * ctpMarketApi.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_DOMAIN_COMPONENTS_CTPMARKETAPI_CTPMARKETAPI_H_
#define WORKSPACE_MARKET_DOMAIN_COMPONENTS_CTPMARKETAPI_CTPMARKETAPI_H_

#include <set>
#include <string>
#include <vector>

#include "common/self/dci/Role.h"
#include "market/domain/components/ctpMarketApi/marketTimeState.h"
#include "market/infra/recer/ctpRecer.h"
#include "common/extern/ctp/inc/ThostFtdcMdApi.h"
#include "common/self/utils.h"

struct MD_InstrumentIDs
{
    pthread_mutex_t sm_mutex;
    std::set<utils::InstrumtntID, utils::InstrumtntIDSortCriterion> instrumentIDs;
};

class CtpMarketBaseApi
{
public:
    CtpMarketBaseApi() {};
    CtpMarketBaseApi(CThostFtdcMdApi* _m_pApi) {};

    CThostFtdcMdApi *CreateFtdcMdApi(const char *pszFlowPath = "");

    const char *GetApiVersion();

    void Release();

    void Init();

    int Join();

    const char *GetTradingDay();

    void RegisterFront(char *pszFrontAddress);

    void RegisterNameServer(char *pszNsAddress);

    void RegisterFensUserInfo(CThostFtdcFensUserInfoField * pFensUserInfo);

    void RegisterSpi(CThostFtdcMdSpi *pSpi);

    // 订阅合约
    int SubscribeMarketData(std::vector<utils::InstrumtntID> const & nameVec);

    // 退订合约
    int UnSubscribeMarketData(std::vector<utils::InstrumtntID> const & nameVec);

    int SubscribeForQuoteRsp(char *ppInstrumentID[], int nCount);

    int UnSubscribeForQuoteRsp(char *ppInstrumentID[], int nCount);

    // 登陆请求
    int ReqUserLogin(void);

    // 登出请求
    int ReqUserLogout(void);

    MD_InstrumentIDs md_InstrumentIDs;
private:
    int nRequestID = 0;
};

enum MARKET_LOGIN_STATE
{
    LOGIN_STATE = 1,
    LOGOUT_STATE = 2
};

struct CtpMarketApi
{
public:
    bool init();

    void login();

    void logout();

    // 从trader端获取所有可交易合约
    int reqInstrumentsFromTrader(void);

    // 从本地构建交易合约
    int reqInstrumentsFromLocal(void);

    // 从本地构建交易合约
    int reqInstrumentsFromStrategy(void);

    void runLogInAndLogOutAlg();

    CtpMarketBaseApi* marketApi;

    // 获取合约信息来源
    std::string getInstrumentsFrom(void);

    // 获取market登录登出状态
    MARKET_LOGIN_STATE getMarketLoginState(void);

    bool release();

    USE_ROLE(MarketTimeState);
private:
    std::string instrumentFrom;
    MarketSpi *marketSpi;
    MARKET_LOGIN_STATE login_state = LOGOUT_STATE;
};


#endif /* WORKSPACE_MARKET_DOMAIN_COMPONENTS_CTPMARKETAPI_CTPMARKETAPI_H_ */
