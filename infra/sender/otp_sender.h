/*
 * ctp_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_OTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_OTPSENDER_H_

#include "common/extern/otp/inc/mds_api/mds_async_api.h"
#include "common/self/utils.h"
#include "market/infra/recer/otp_recer.h"
#include "market/infra/sender/send_api.h"

struct OtpSender : SendApi {
 public:
  OtpSender(void);
  ~OtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumentID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumentID> const &name_vec, int request_id = 0);
  bool LossConnection();
  bool Release();

  static OtpMarketSpi *market_spi;

 private:
  bool Init(void);

  void UpdateInstrumentInfoFromMarket();

  MdsAsyncApiContextT *async_context_;
  MdsAsyncApiChannelT *async_channel_;
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_XTPSENDER_H_ */
