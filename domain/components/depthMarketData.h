#ifndef DEPTH_MARKET_DATA_H
#define DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/self/basetype.h"
#include "market/domain/components/depthMarketData.h"

struct marketData {
 public:
  marketData();
  ~marketData(){};
  bool isValidTickData(CThostFtdcDepthMarketDataField *pD);
  bool insertInsExchPair(const std::string &ins, const std::string &exch);
  bool clearInsExchPair(void);
  bool showInsExchPair(void);
  std::string findExchange(std::string ins);
  double max2zero(double num);

  bool getLocalTime(char *t_arr);
  bool getLocalTime(long &stamp);
  bool getAssemblingTime(char *t_arr, CThostFtdcDepthMarketDataField *pD);

 private:
  std::map<std::string, std::string> md_Instrument_Exhange;
  bool printNetworkDelay = false;
};

#endif