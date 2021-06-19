#ifndef STORE_DEPTH_MARKET_DATA_H
#define STORE_DEPTH_MARKET_DATA_H

#include <vector>
#include <string>
#include <map>

#include "common/self/basetype.h"
#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"

#include "market/domain/components/depthMarketData.h"


struct loadData :public marketData{
public:
    loadData();
    ~loadData() {};
    void LoadDepthMarketDataToCsv(CThostFtdcDepthMarketDataField * pD);
    void FormDepthMarketData2Stringflow(CThostFtdcDepthMarketDataField * pD);

    bool ClassifyContractFiles(void);
    bool MoveContractToFolder(std::string contractName, std::string exchangeName);
private:
    std::string history_tick_folder;
    char dataflow[1000];
    char titleflow[320]={"InstrumentID,TradingDay,UpdateTime,LastPrice,BidPrice1,BidVolume1,AskPrice1,AskVolume1,Volume,Turnover,OpenInterest,UpperLimitPrice,LowerLimitPrice,OpenPrice,PreSettlementPrice,PreClosePrice,PreOpenInterest"};
};

#endif