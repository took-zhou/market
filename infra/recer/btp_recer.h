/*
 * ctp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_BTPRECER_H_
#define WORKSPACE_MARKET_INFRA_BTPRECER_H_
#include "common/extern/btp/inc/btp_market_api.h"
#include "common/self/utils.h"

class BtpMarketSpi : public btp::api::MarketSpi {
 public:
  BtpMarketSpi() {}
  ~BtpMarketSpi() {}

  void OnRspUserLogin(const BtpLoginLogoutStruct *login_info);
  void OnRspUserLogout(const BtpLoginLogoutStruct *login_info);
  void OnDepthMarketData(const BtpMarketDataStruct *market_data);
  void OnRspInstrumentInfo(BtpInstrumentInfo *instrument_info);
};

#endif /* WORKSPACE_MARKET_INFRA_CTPRECER_H_ */
