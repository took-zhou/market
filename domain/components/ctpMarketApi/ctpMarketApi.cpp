/*
 * ctpMarketApi.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/infra/recer/ctpRecer.h"
#include "market/infra/recerSender.h"
#include "market/domain/components/ctpMarketApi/ctpMarketApi.h"
#include "market/domain/marketService.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include <string>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <algorithm>

#include "common/self/semaphorePart.h"
extern GlobalSem globalSem;
CThostFtdcMdApi* _m_pApi;

CThostFtdcMdApi* CtpMarketBaseApi::CreateFtdcMdApi(const char *pszFlowPath)
{
    INFO_LOG("_m_pApi init ok");
    _m_pApi = CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath, true, true);
    return _m_pApi;
}

const char* CtpMarketBaseApi::GetApiVersion()
{
    return _m_pApi->GetApiVersion();
}

void CtpMarketBaseApi::Release()
{
    // 释放UserApi
    _m_pApi->Release();
    pthread_mutex_destroy(&(md_InstrumentIDs.sm_mutex));
    md_InstrumentIDs.instrumentIDs.clear();
    return;
}

void CtpMarketBaseApi::Init()
{
    _m_pApi->Init();
    std::string semName = "market_init";
    globalSem.addOrderSem(semName);
    md_InstrumentIDs.instrumentIDs.clear();
    pthread_mutex_init(&(md_InstrumentIDs.sm_mutex), NULL);  
    INFO_LOG("m_pApi->Init send ok!");
    return;
}

int CtpMarketBaseApi::Join()
{
    return 0;
}

const char* CtpMarketBaseApi::GetTradingDay()
{
    return _m_pApi->GetTradingDay();
}

void CtpMarketBaseApi::RegisterFront(char *pszFrontAddress)
{
    if (pszFrontAddress == nullptr)
    {
        ERROR_LOG("pszFrontAddress is nullptr");
        return;
    }

    _m_pApi->RegisterFront(pszFrontAddress);
    INFO_LOG("RegisterFront [%s] ok",pszFrontAddress);
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
    if (pSpi == nullptr)
    {
        ERROR_LOG("pSpi is nullptr");
        return;
    }

    _m_pApi->RegisterSpi(pSpi);
    return;
}

// 先判断ppInstrumentID是否在md_InstrumentIDs
// 在 忽略; 不在 订阅
int CtpMarketBaseApi::SubscribeMarketData(std::vector<utils::InstrumtntID> const & nameVec)
{
    int result = 0;
    int md_num = 0;

    if (nameVec.size() > 500)
    {
        WARNING_LOG("too much instruments to Subscription.");
        return result;
    }

    int ik = pthread_mutex_lock(&(md_InstrumentIDs.sm_mutex));
    char **ppInstrumentID2 = new char*[5000];

    for (int i = 0; i < nameVec.size(); i++)
    {
        if (md_InstrumentIDs.instrumentIDs.find(nameVec[i]) == end(md_InstrumentIDs.instrumentIDs))
        {
            ppInstrumentID2[md_num] = const_cast<char *>(nameVec[i].ins.c_str());
            md_InstrumentIDs.instrumentIDs.insert(nameVec[i]);
            md_num++;
        }
    }

    if (md_num > 0)
    {
        result = _m_pApi->SubscribeMarketData(ppInstrumentID2, md_num);
        if( result == 0 )
        {
            INFO_LOG("Subscription request ......Send a success, total number: %d", md_num);
        }
        else
        {
            INFO_LOG("Subscription request ......Failed to send, error serial number=[%d]", result);
        }
    }
    else
    {
        INFO_LOG("no instrument need to Subscription.");
    }

    delete[] ppInstrumentID2;
    ik = pthread_mutex_unlock(&(md_InstrumentIDs.sm_mutex));

    return result;
}

int CtpMarketBaseApi::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const & nameVec)
{
    int result = 0;
    int md_num = 0;

    if (nameVec.size() > 500)
    {
        WARNING_LOG("too much instruments to unSubscription.");
        return result;
    }

    int ik = pthread_mutex_lock(&(md_InstrumentIDs.sm_mutex));
    char **ppInstrumentID2 = new char*[5000];

    for (int i = 0; i < nameVec.size(); i++)
    {
        if (md_InstrumentIDs.instrumentIDs.find(nameVec[i]) != end(md_InstrumentIDs.instrumentIDs))
        {
            ppInstrumentID2[md_num] = const_cast<char *>(nameVec[i].ins.c_str());
            md_InstrumentIDs.instrumentIDs.erase(nameVec[i]);
            md_num++;
        }
    }

    if (md_num > 0)
    {
        result = _m_pApi->UnSubscribeMarketData(ppInstrumentID2, md_num);
        if( result == 0 )
        {
            INFO_LOG("unSubscription request ......Send a success, total number: %d", md_num);
        }
        else
        {
            INFO_LOG("unSubscription request ......Failed to send, error serial number=[%d]", result);
        }
    }
    else
    {
        INFO_LOG("no instrument need to unSubscription.");
    }

    delete[] ppInstrumentID2;
    ik = pthread_mutex_unlock(&(md_InstrumentIDs.sm_mutex));

    return result;
}

int CtpMarketBaseApi::SubscribeForQuoteRsp(char *ppInstrumentID[], int nCount)
{
    return 0;
}

int CtpMarketBaseApi::UnSubscribeForQuoteRsp(char *ppInstrumentID[], int nCount)
{
    return 0;
}

int CtpMarketBaseApi::ReqUserLogin()
{
    CThostFtdcReqUserLoginField reqUserLogin = {0};;
    auto& jsonCfg = utils::JsonConfig::getInstance();

    const std::string username = jsonCfg.getConfig("common","user").get<std::string>();
    const std::string userID = jsonCfg.getDeepConfig("users", username, "UserID");
    const std::string brokerID = jsonCfg.getDeepConfig("users", username, "BrokerID");
    const std::string passWord = jsonCfg.getDeepConfig("users", username, "Password");
    strcpy(reqUserLogin.BrokerID, brokerID.c_str());
    strcpy(reqUserLogin.UserID, userID.c_str());
    strcpy(reqUserLogin.Password, passWord.c_str());

    std::string semName = "market_login";
    globalSem.addOrderSem(semName);

    int result = _m_pApi->ReqUserLogin(&reqUserLogin, nRequestID++);
    INFO_LOG("ReqUserLogin send result is [%d]",result);
    return result;
}

int CtpMarketBaseApi::ReqUserLogout()
{
    CThostFtdcUserLogoutField reqUserLogout = {0};
    auto& jsonCfg = utils::JsonConfig::getInstance();

    const std::string username = jsonCfg.getConfig("common","user").get<std::string>();
    const std::string userID = jsonCfg.getDeepConfig("users", username, "UserID").get<std::string>();
    const std::string brokerID = jsonCfg.getDeepConfig("users", username, "BrokerID").get<std::string>();
    strcpy(reqUserLogout.UserID, userID.c_str());
    strcpy(reqUserLogout.BrokerID, brokerID.c_str());

    std::string semName = "market_logout";
    globalSem.addOrderSem(semName);

    int result = _m_pApi->ReqUserLogout(&reqUserLogout, nRequestID++);
    INFO_LOG("ReqUserLogout send result is [%d]",result);
    return result;
}

bool CtpMarketApi::init()
{
    INFO_LOG("begin CtpMarketApi init");
    marketApi = new CtpMarketBaseApi;
    auto& jsonCfg = utils::JsonConfig::getInstance();
    const std::string conPath = jsonCfg.getConfig("market","ConRelativePath").get<std::string>();
    utils::creatFolder(conPath);
    marketApi->CreateFtdcMdApi(conPath.c_str());
    INFO_LOG("ctp version: %s", marketApi->GetApiVersion());

    marketSpi = new MarketSpi();
    marketApi->RegisterSpi(marketSpi);

    const std::string username = jsonCfg.getConfig("common","user").get<std::string>();
    std::string frontaddr = jsonCfg.getDeepConfig("users", username, "FrontMdAddr").get<std::string>();

    marketApi->RegisterFront(const_cast<char *>(frontaddr.c_str()));

    marketApi->Init();

    std::string compile_time = utils::GetCompileTime();
    jsonCfg.writeConfig("market", "version", compile_time);
    INFO_LOG("program last build at %s.", compile_time.c_str());

    std::string semName = "market_init";
    /*在这个地方加一个登录登出的优化*/
    globalSem.waitSemBySemName(semName);
    globalSem.delOrderSem(semName);

    sleep(1);
    INFO_LOG("market init ok.");
}

int CtpMarketApi::reqInstrumentsFromTrader(void)
{
    market_trader::message reqMsg;
    auto reqInstrument = reqMsg.mutable_qry_instrument_req();
    reqInstrument->set_identity("all");
    std::string reqStr;
    reqMsg.SerializeToString(&reqStr);

    auto& recerSender = RecerSender::getInstance();
    recerSender.ROLE(Sender).ROLE(ProxySender).send("market_trader.QryInstrumentReq", reqStr.c_str());
}

int CtpMarketApi::reqInstrumentsFromLocal(void)
{
    utils::InstrumtntID instrumtntID;
    vector<utils::InstrumtntID> ins_vec;
    instrumtntID.exch = "DCE";
    instrumtntID.ins = "c2101";
    ins_vec.push_back(instrumtntID);
    instrumtntID.ins = "c2103";
    ins_vec.push_back(instrumtntID);
    instrumtntID.ins = "c2105";
    ins_vec.push_back(instrumtntID);

    marketApi->SubscribeMarketData(ins_vec);
}

int CtpMarketApi::reqInstrumentsFromStrategy(void)
{
    auto& marketSer = MarketService::getInstance();
    auto instrumentVec = marketSer.ROLE(controlPara).getInstrumentList();

    if (instrumentVec.size() != 0)
    {
        marketApi->SubscribeMarketData(instrumentVec);
    }
}

std::string CtpMarketApi::getInstrumentsFrom()
{
    return instrumentFrom;
}

bool CtpMarketApi::release()
{
    INFO_LOG("Is going to release marketApi.");
    marketApi->Release();
    delete marketApi;
    marketApi = nullptr;

    // 释放UserSpi实例
    if (marketSpi)
    {
        delete marketSpi;
        marketSpi = NULL;
    }
}

void CtpMarketApi::login()
{
    INFO_LOG("login time, is going to login.");
    marketApi->ReqUserLogin();
    std::string semName = "market_login";
    globalSem.waitSemBySemName(semName);
    globalSem.delOrderSem(semName);

    login_state = LOGIN_STATE;
}

// ctp market登出有异常，做特殊处理
void CtpMarketApi::logout()
{
    INFO_LOG("logout time, is going to logout.");
    marketApi->ReqUserLogout();

    auto logOutReqTimeOutFunc = [&]()
    {
        marketSpi->OnRspUserLogout();
    };

    auto& timerPool = TimeoutTimerPool::getInstance();
    timerPool.addTimer(MARKET_LOGOUT_TIMER, logOutReqTimeOutFunc, MARKET_LOGOUT_TIMEOUT);

    std::string semName = "market_logout";
    globalSem.waitSemBySemName(semName);
    globalSem.delOrderSem(semName);

    timerPool.killTimerByName(MARKET_LOGOUT_TIMER);
    login_state = LOGOUT_STATE;
}

MARKET_LOGIN_STATE CtpMarketApi::getMarketLoginState(void)
{
    return login_state;
}

void CtpMarketApi::runLogInAndLogOutAlg()
{
    while(1)
    {
        if (ROLE(MarketTimeState).output.status == LOGIN_TIME && login_state == LOGOUT_STATE)
        {
            this->init();
            if (ROLE(MarketTimeState).output.status == LOGIN_TIME)
            {
                this->login();
            }
            else
            {
                this->release();
            }
        }
        else if (ROLE(MarketTimeState).output.status == LOGOUT_TIME && login_state == LOGIN_STATE)
        {
            this->logout();
        }
        sleep(1);
    }
    return;
}
