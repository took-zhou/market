/*
 * ftp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_FTPRECER_H_
#define WORKSPACE_MARKET_INFRA_FTPRECER_H_
#include "common/extern/ftp/inc/ftp_market_api.h"

class FtpMarketSpi : public ftp::api::MarketSpi {
 public:
  FtpMarketSpi() {}
  virtual ~FtpMarketSpi() {}

  void OnRspUserLogin(const FtpLoginLogoutStruct *login_info);
  void OnRspUserLogout(const FtpLoginLogoutStruct *login_info);
  void OnDepthMarketData(const FtpMarketDataStruct *market_data);
  void OnRspAllInstrumentInfo(FtpInstrumentInfo *ticker_info);
};

#endif /* WORKSPACE_MARKET_INFRA_FTPRECER_H_ */
