/*
 * ytp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_YTPRECER_H_
#define WORKSPACE_MARKET_INFRA_YTPRECER_H_
#include "common/extern/ytp/inc/ytp_market_api.h"

class YtpMarketSpi : public ytp::api::MarketSpi {
 public:
  YtpMarketSpi() {}
  virtual ~YtpMarketSpi() {}

  void OnRspUserLogin(const YtpLoginLogoutStruct *login_info);
  void OnRspUserLogout(const YtpLoginLogoutStruct *login_info);
  void OnDepthMarketData(const YtpMarketDataStruct *market_data);
  void OnRspAllInstrumentInfo(YtpInstrumentInfo *ticker_info);
  void OnFrontDisconnected(int reason);
  bool GetFrontDisconnected(void);

 private:
  bool front_disconnected_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_YTPRECER_H_ */
