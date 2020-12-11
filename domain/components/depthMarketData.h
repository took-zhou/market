#ifndef DEPTH_MARKET_DATA_H
#define DEPTH_MARKET_DATA_H

#include <vector>
#include <string>
#include <map>

#include "common/self/basetype.h"
#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "market/domain/components/depthMarketData.h"


struct marketData{
public:
    marketData();
    ~marketData() {};
    bool isValidTickData(CThostFtdcDepthMarketDataField * pD);
    bool insertInsExchPair(const std::string &ins, const std::string &exch);
    std::string findExchange(std::string ins);
    double max2zero(double num);

    bool getLocalTime(char *t_arr);
    bool getLocalTime(long &stamp);
private:
    std::map<std::string, std::string> md_Instrument_Exhange;
};

#endif