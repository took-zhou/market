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
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec);
  bool ReqInstrumentInfo(const utils::InstrumtntID &ins);
  bool LossConnection();

  static XTP::API::QuoteApi *quote_api;
  static XtpQuoteSpi *quote_spi;

 private:
  bool Init(void);
  bool Release(void);

  std::string con_path_ = "";
  int request_id_ = 0;
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_XTPSENDER_H_ */
