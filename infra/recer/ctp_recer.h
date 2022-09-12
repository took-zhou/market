/*
 * ctp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_CTPRECER_H_
#define WORKSPACE_MARKET_INFRA_CTPRECER_H_
#include "common/extern/ctp/inc/ThostFtdcMdApi.h"
#include "common/self/utils.h"

class CtpMarketSpi : public CThostFtdcMdSpi {
 public:
  ~CtpMarketSpi() {}
  void OnFrontConnected();

  void OnFrontDisconnected(int nReason);

  void OnHeartBeatWarning(int nTimeLapse);

  void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

  void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

  // market端ctp登出没有反馈，主动调用反馈接口
  void OnRspUserLogout(void);

  void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

  void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                          bool bIsLast);

  void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                            bool bIsLast);

  void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                           bool bIsLast);

  void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                             bool bIsLast);

  void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

  void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

  bool front_disconnected = false;
};

#endif /* WORKSPACE_MARKET_INFRA_CTPRECER_H_ */
