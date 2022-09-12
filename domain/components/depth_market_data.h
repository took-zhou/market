#ifndef DEPTH_MARKET_DATA_H
#define DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/extern/xtp/inc/xquote_api_struct.h"

#include "market/domain/components/depth_market_data.h"

struct MarketData {
 public:
  MarketData();
  ~MarketData(){};
  bool IsValidTickData(CThostFtdcDepthMarketDataField *pD);
  bool IsValidTickData(XTPMD *pD);

  bool InsertInsExchPair(const std::string &ins, const std::string &exch);
  bool ClearInsExchPair(void);
  bool ShowInsExchPair(void);
  std::string FindExchange(const std::string &ins);
  double Max2zero(double num);

  bool get_local_time(char *t_arr);
  bool get_local_time(long &stamp);
  bool get_assembling_time(char *t_arr, CThostFtdcDepthMarketDataField *pD);
  bool get_assembling_time(char *t_arr, XTPMD *pD);

 private:
  std::map<std::string, std::string> instrument_exchange_map;
};

#endif