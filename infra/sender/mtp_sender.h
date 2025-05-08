/*
 * mtp_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_MTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_MTPSENDER_H_

#include "common/extern/mtp/inc/mtp_market_api.h"
#include "common/self/utils.h"
#include "market/infra/recer/mtp_recer.h"
#include "market/infra/sender/send_api.h"

struct MtpSender : SendApi {
 public:
  MtpSender(void);
  ~MtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumentID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumentID> const &name_vec, int request_id = 0);
  bool LossConnection();
  bool Release();

  static mtp::api::MarketApi *market_api;
  static MtpMarketSpi *market_spi;

 private:
  bool Init(void);

  std::string con_path_ = "";
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_MTPSENDER_H_ */
