/*
 * ftp_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_FTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_FTPSENDER_H_

#include "common/extern/ftp/inc/ftp_market_api.h"
#include "common/self/utils.h"
#include "market/infra/recer/ftp_recer.h"
#include "market/infra/sender/send_api.h"

struct FtpSender : SendApi {
 public:
  FtpSender(void);
  ~FtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool LossConnection();
  bool Release();

  static ftp::api::MarketApi *market_api;
  static FtpMarketSpi *market_spi;

 private:
  bool Init(void);
  std::string con_path_ = "";
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_FTPSENDER_H_ */
