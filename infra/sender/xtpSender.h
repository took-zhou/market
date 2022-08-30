/*
 * ctpSender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_XTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_XTPSENDER_H_

#include "common/extern/xtp/inc/xtp_quote_api.h"
#include "common/self/utils.h"
#include "market/infra/recer/xtpRecer.h"
#include "market/infra/sender/sendApi.h"

struct XtpSender : SendApi {
 public:
  XtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool ReqInstrumentInfo(const utils::InstrumtntID &ins);
  bool LossConnection();

  static XTP::API::QuoteApi *quoteApi;
  static XtpQuoteSpi *quoteSpi;

 private:
  bool init(void);
  bool release(void);

  std::string conPath = "";
  int nRequestID = 0;
  bool isInit = false;
};

#endif /* WORKSPACE_MARKET_INFRA_XTPSENDER_H_ */
