/*
 * ctpSender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_CTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_CTPSENDER_H_

#include "common/extern/ctp/inc/ThostFtdcMdApi.h"
#include "common/self/utils.h"
#include "market/infra/recer/ctpRecer.h"
#include "market/infra/sender/sendApi.h"

struct CtpSender : SendApi {
 public:
  CtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool ReqInstrumentInfo(const utils::InstrumtntID &ins);
  bool LossConnection();

  static CThostFtdcMdApi *marketApi;
  static CtpMarketSpi *marketSpi;

 private:
  bool init(void);
  bool release(void);

  std::string conPath = "";
  int nRequestID = 0;
  bool isInit = false;
};

#endif /* WORKSPACE_MARKET_INFRA_CTPSENDER_H_ */
