#ifndef BUILD_CONTRACT_DATA_POOL_H
#define BUILD_CONTRACT_DATA_POOL_H



#include <ThostFtdcUserApiStruct.h>



void BuildContractArray(void);
void InsertDataToContractPool(MARKET_MSG * sM, CThostFtdcDepthMarketDataField * pD);

#endif