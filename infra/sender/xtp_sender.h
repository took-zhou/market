/*
 * ctp_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_XTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_XTPSENDER_H_

#include "common/extern/xtp/inc/xtp_quote_api.h"
#include "common/self/utils.h"
#include "market/infra/recer/xtp_recer.h"
#include "market/infra/sender/send_api.h"

struct XtpSender : SendApi {
 public:
  XtpSender(void);
  ~XtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool LossConnection();
  bool Release();

  static XTP::API::QuoteApi *quote_api;
  static XtpQuoteSpi *quote_spi;

 private:
  void UpdateInstrumentInfoFromMarket();
  bool Init(void);

  std::string con_path_ = "";
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_XTPSENDER_H_ */
