/*
 * gtp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_GTPRECER_H_
#define WORKSPACE_MARKET_INFRA_GTPRECER_H_
#include "common/extern/gtp/inc/gtp_market_api.h"

class GtpMarketSpi : public gtp::api::MarketSpi {
 public:
  GtpMarketSpi() {}
  virtual ~GtpMarketSpi() {}

  void OnRspUserLogin(const GtpLoginLogoutStruct *login_info);
  void OnRspUserLogout(const GtpLoginLogoutStruct *login_info);
  void OnDepthMarketData(const GtpMarketDataStruct *market_data);
  void OnRspAllInstrumentInfo(GtpInstrumentInfo *ticker_info);
  void OnFrontDisconnected(int reason);
  bool GetFrontDisconnected(void);

 private:
  bool front_disconnected_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_GTPRECER_H_ */
