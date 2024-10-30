#ifndef DEPTH_MARKET_DATA_H
#define DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/extern/gtp/inc/gtp_market_struct.h"
#include "common/extern/mtp/inc/mtp_market_struct.h"
#include "common/extern/otp/inc/mds_global/mds_base_model.h"
#include "common/extern/xtp/inc/xquote_api_struct.h"

struct MarketData {
 public:
  MarketData();
  ~MarketData(){};
  bool IsValidTickData(CThostFtdcDepthMarketDataField *p_d);
  bool IsValidTickData(XTPMD *p_d);
  bool IsValidTickData(MdsMktDataSnapshotT *p_d);
  bool IsValidTickData(GtpMarketDataStruct *p_d);
  bool IsValidTickData(MtpMarketDataStruct *p_d);

  double Max2zero(double num);

  bool GetAssemblingTime(char *t_arr, CThostFtdcDepthMarketDataField *p_d);
  bool GetAssemblingTime(char *t_arr, XTPMD *p_d);
  bool GetAssemblingTime(char *t_arr, MdsMktDataSnapshotT *p_d);
  bool GetAssemblingTime(char *t_arr, GtpMarketDataStruct *p_d);
  bool GetAssemblingTime(char *t_arr, MtpMarketDataStruct *p_d);

 private:
  std::map<std::string, std::string> instrument_exchange_map_;
};

#endif