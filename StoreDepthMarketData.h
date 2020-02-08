#ifndef STORE_DEPTH_MARKET_DATA_H
#define STORE_DEPTH_MARKET_DATA_H



#include <ThostFtdcUserApiStruct.h>
#include <vector>


void LoadDepthMarketDataToMysql(void);
void LoadDepthMarketDataToCsv(CThostFtdcDepthMarketDataField * pD);
void LoadTradingContracts(vector<string> *vec);

#endif