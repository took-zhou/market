#ifndef DEPTH_MARKET_DATA_H
#define DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/extern/xtp/inc/xquote_api_struct.h"

struct MarketData {
 public:
  MarketData();
  ~MarketData(){};
  bool IsValidTickData(CThostFtdcDepthMarketDataField *p_d);
  bool IsValidTickData(XTPMD *p_d);

  double Max2zero(double num);

  bool GetLocalTime(char *t_arr);
  bool GetLocalTime(long &stamp);
  bool GetAssemblingTime(char *t_arr, CThostFtdcDepthMarketDataField *p_d);
  bool GetAssemblingTime(char *t_arr, XTPMD *p_d);

 private:
  std::map<std::string, std::string> instrument_exchange_map_;
};

#endif