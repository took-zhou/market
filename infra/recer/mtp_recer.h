/*
 * mtp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_MTPRECER_H_
#define WORKSPACE_MARKET_INFRA_MTPRECER_H_
#include "common/extern/mtp/inc/mtp_market_api.h"

class MtpMarketSpi : public mtp::api::MarketSpi {
 public:
  MtpMarketSpi() {}
  virtual ~MtpMarketSpi() {}

  void OnRspUserLogin(const MtpLoginLogoutStruct *login_info);
  void OnRspUserLogout(const MtpLoginLogoutStruct *login_info);
  void OnDepthMarketData(const MtpMarketDataStruct *market_data);
  void OnRspAllInstrumentInfo(MtpInstrumentInfo *ticker_info);
  void OnFrontDisconnected(int reason);
  bool GetFrontDisconnected(void);

 private:
  bool front_disconnected_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_MTPRECER_H_ */
