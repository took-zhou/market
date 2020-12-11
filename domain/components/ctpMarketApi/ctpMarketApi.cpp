/*
 * ctpMarketApi.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/infra/recer/ctpRecer.h"
#include "market/infra/recerSender.h"
#include "market/domain/components/ctpMarketApi/ctpMarketApi.h"
#include "market/domain/components/ctpMarketApi/marketLoginState.h"
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

CThostFtdcMdApi* CtpMarketBaseApi::CreateFtdcMdApi(const char *pszFlowPath)
{
    INFO_LOG("_m_pApi init ok");
    _m_pApi = CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath);
    return _m_pApi;
}

const char* CtpMarketBaseApi::GetApiVersion()
{
    return CThostFtdcMdApi::GetApiVersion();
}

void CtpMarketBaseApi::Release()
{
    _m_pApi->Release();
    return;
}

void CtpMarketBaseApi::Init()
{
    _m_pApi->Init();
    std::string semName = "market_init";
    globalSem.addOrderSem(semName);
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
    INFO_LOG("lslt");
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

    int ik = pthread_mutex_lock(&md_InstrumentIDs.sm_mutex);
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

    delete ppInstrumentID2;
    ik = pthread_mutex_unlock(&md_InstrumentIDs.sm_mutex);

    return result;
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

int CtpMarketBaseApi::ReqUserLogin()
{
    CThostFtdcReqUserLoginField reqUserLogin = {0};;
    auto& jsonCfg = utils::JsonConfig::getInstance();
    const std::string userID = jsonCfg.getConfig("market","UserID").get<std::string>();
    const std::string brokerID = jsonCfg.getConfig("market","BrokerID").get<std::string>();
    const std::string passWord = jsonCfg.getConfig("market","Password").get<std::string>();
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
    const std::string userID = jsonCfg.getConfig("market","UserID").get<std::string>();
    const std::string brokerID = jsonCfg.getConfig("market","BrokerID").get<std::string>();
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
    sem_t sem;
    sem_init(&sem,0,0);
    INFO_LOG("begin CtpMarketApi init");
    marketApi = new CtpMarketBaseApi;
    auto& jsonCfg = utils::JsonConfig::getInstance();
    const std::string conPath = jsonCfg.getConfig("market","ConRelativePath").get<std::string>();
    utils::creatFolder(conPath);
    marketApi->CreateFtdcMdApi(conPath.c_str());

    auto& marketSpi = MarketSpi::getInstance();
    marketApi->RegisterSpi(&marketSpi);

    std::string frontaddr = jsonCfg.getConfig("market","FrontMdAddr").get<std::string>();
    marketApi->RegisterFront(const_cast<char *>(frontaddr.c_str()));
}

int CtpMarketApi::reqInstrumentsFromTrader(void)
{
    market_trader::message reqMsg;
    auto reqInstrument = reqMsg.mutable_qry_instrument_req();
    reqInstrument->set_identity("all");
    std::string reqStr;
    reqMsg.SerializeToString(&reqStr);
    INFO_LOG("reqStr: %s", reqStr.c_str());
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

std::string CtpMarketApi::getInstrumentsFrom()
{
    return instrumentFrom;
}

void CtpMarketApi::runLogInAndLogOutAlg()
{
    auto loginState = MarketLoginState();
    thread t(&MarketLoginState::update, std::ref(loginState));
    t.detach();

    while(1)
    {
        if (loginState.output.status == LOGIN_TIME)
        {
            this->init();
            INFO_LOG("during login time.");
            marketApi->Init();
            std::string semName = "market_init";
            globalSem.waitSemBySemName(semName);
            globalSem.delOrderSem(semName);
            INFO_LOG("market init ok.");

            marketApi->ReqUserLogin();
            semName = "market_login";
            globalSem.waitSemBySemName(semName);
            globalSem.delOrderSem(semName);
            INFO_LOG("wait for logout time.");

            while(1)
            {
                if (loginState.output.status == LOGOUT_TIME)
                {
                    std::string semName = "market_logout";
                    globalSem.addOrderSem(semName);
                    auto& marketSpi = MarketSpi::getInstance();
                    marketSpi.OnRspUserLogout();
                    semName = "market_logout";
                    globalSem.waitSemBySemName(semName);
                    globalSem.delOrderSem(semName);
                    INFO_LOG("logout time, is going to logout.");

                    break;
                }
                else
                {
                    sleep(1);
                }
            }
        }
        else
        {
            sleep(1);
        }
    }
    return;
}
