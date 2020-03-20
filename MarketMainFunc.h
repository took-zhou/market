#ifndef MARKET_MAIN_FUNC_H
#define MARKET_MAIN_FUNC_H

// CTP头文件
#include <ThostFtdcTraderApi.h>
#include <ThostFtdcMdApi.h>
#include <ThostFtdcUserApiDataType.h>
#include <ThostFtdcUserApiStruct.h>

#include <pthread.h>

#define CONSOLE_OUT         0u
#define MARKET_BUF_SIZE     100u
#define MARKET_BUF_ID       229379u


typedef struct market_msg
{
    pthread_mutex_t sm_mutex;
    CThostFtdcDepthMarketDataField datafield[MARKET_BUF_SIZE];
}MARKET_MSG;

class CMdHandler : public CThostFtdcMdSpi {

public:
    //共享内存结构体指针
    MARKET_MSG *share_msg;
    
    // 构造函数，需要一个有效的指向CThostFtdcMduserApi实例的指针
    CMdHandler(CThostFtdcMdApi *pUserApi);// : m_pUserMdApi(pUserApi){}
    ~CMdHandler(){}

    // 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
    virtual void OnFrontConnected();

    virtual void OnHeartBeatWarning(int nTimeLapse);

    void ReqUserLogin(void);

    // 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void ReqUserLogout(void);
    
    ///登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);    

    //收行情
    void SubscribeMarketData(void);

    ///订阅行情应答
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///深度行情通知
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    ///订阅询价请求
    void SubscribeForQuoteRsp(void);

    //订阅询价应答
    virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
        int nRequestID, bool bIsLast);


    ///询价通知
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);  

    //登出时间段
    void WaitLogoutTime(void);  

private:
    // 指向CThostFtdcMduserApi实例的指针
    CThostFtdcMdApi *m_pUserMdApi;
    int reConnect; 

};


#endif

