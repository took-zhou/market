/*
 * ctpRecer.cpp
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#include "market/infra/recer/ctpRecer.h"
#include "common/extern/libgo/libgo/libgo.h"
#include "common/extern/log/log.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/infra/define.h"

extern GlobalSem globalSem;

extern co_chan<MsgStruct> ctpMsgChan;

void MarketSpi::OnFrontConnected() {
  INFO_LOG("OnFrontConnected():is excuted...");
  // 在登出后系统会重新调用OnFrontConnected，这里简单判断并忽略第1次之后的所有调用。
  if (reConnect++ == 0) {
    std::string semName = "market_init";
    globalSem.postSemBySemName(semName);
    INFO_LOG("post sem of [%s]", semName.c_str());
  }

  INFO_LOG("reConnect:%d.", reConnect);
}

void MarketSpi::OnFrontDisconnected(int nReason) { ERROR_LOG("OnFrontDisconnected, ErrorCode:%#x", nReason); }

void MarketSpi::OnHeartBeatWarning(int nTimeLapse) { ERROR_LOG("OnHeartBeatWarning  %d!", nTimeLapse); }

void MarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  MsgStruct msgStruct;
  msgStruct.sessionName = "ctp";
  msgStruct.msgName = "LoginInfo";
  msgStruct.ctpMsg = pRspInfo;

  globalSem.addOrderSem(msgStruct.msgName);
  ctpMsgChan << msgStruct;
  globalSem.waitSemBySemName(msgStruct.msgName);
}

void MarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  MsgStruct msgStruct;
  msgStruct.sessionName = "ctp";
  msgStruct.msgName = "LogoutInfo";
  msgStruct.ctpMsg = pRspInfo;

  globalSem.addOrderSem(msgStruct.msgName);
  ctpMsgChan << msgStruct;
  globalSem.waitSemBySemName(msgStruct.msgName);
}

void MarketSpi::OnRspUserLogout(void) {
  CThostFtdcRspInfoField field;
  field.ErrorID = 0;
  strcpy(field.ErrorMsg, "force logout");

  MsgStruct msgStruct;
  msgStruct.sessionName = "ctp";
  msgStruct.msgName = "LogoutInfo";
  msgStruct.ctpMsg = &field;

  globalSem.addOrderSem(msgStruct.msgName);
  ctpMsgChan << msgStruct;
  globalSem.waitSemBySemName(msgStruct.msgName);
}

void MarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
#ifdef BENCH_TEST
  ScopedTimer t("OnRtnDepthMarketData");
#endif
  MsgStruct msgStruct;
  msgStruct.sessionName = "ctp";
  msgStruct.msgName = "OnRtnDepthMarketData";
  msgStruct.ctpMsg = pDepthMarketData;

  globalSem.addOrderSem(msgStruct.msgName);
  ctpMsgChan << msgStruct;
  globalSem.waitSemBySemName(msgStruct.msgName);
}

void MarketSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {}

void MarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void MarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                   bool bIsLast) {}

void MarketSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                                     int nRequestID, bool bIsLast) {}

void MarketSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                                    int nRequestID, bool bIsLast) {}

void MarketSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                                      int nRequestID, bool bIsLast) {}
