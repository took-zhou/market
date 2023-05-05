/*
 * ctp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_OTPRECER_H_
#define WORKSPACE_MARKET_INFRA_OTPRECER_H_
#include "common/extern/otp/inc/mds_api/mds_async_api.h"
#include "common/self/utils.h"

class OtpMarketSpi {
 public:
  OtpMarketSpi() {}
  virtual ~OtpMarketSpi() {}

  void OnRspUserLogin(const std::string *login_info);
  void OnRspUserLogout(const std::string *logout_info);
  void OnRspStockStaticInfo(const MdsStockStaticInfoT *static_info, bool is_last);
  void OnDepthMarketData(const MdsMktDataSnapshotT *market_data);
};

#endif /* WORKSPACE_MARKET_INFRA_CTPRECER_H_ */
