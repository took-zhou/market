/*
 * gtp_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_GTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_GTPSENDER_H_

#include "common/extern/gtp/inc/gtp_market_api.h"
#include "common/self/utils.h"
#include "market/infra/recer/gtp_recer.h"
#include "market/infra/sender/send_api.h"

struct GtpSender : SendApi {
 public:
  GtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool LossConnection();

  static gtp::api::MarketApi *market_api;
  static GtpMarketSpi *market_spi;

 private:
  bool Init(void);
  bool Release(void);

  std::string con_path_ = "";
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_GTPSENDER_H_ */
