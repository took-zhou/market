/*
 * ctp_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_BTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_BTPSENDER_H_

#include "common/extern/btp/inc/btp_market_api.h"
#include "common/self/utils.h"
#include "market/infra/recer/btp_recer.h"
#include "market/infra/sender/send_api.h"

struct BtpSender : SendApi {
 public:
  BtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool LossConnection();

  static btp::api::MarketApi *market_api;
  static BtpMarketSpi *market_spi;

 private:
  bool Init(void);
  bool Release(void);

  std::string con_path_ = "";
  int request_id_ = 0;
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_XTPSENDER_H_ */
