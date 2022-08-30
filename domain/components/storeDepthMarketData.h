#ifndef STORE_DEPTH_MARKET_DATA_H
#define STORE_DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/extern/xtp/inc/xquote_api_struct.h"
#include "common/self/basetype.h"

#include "market/domain/components/depthMarketData.h"

struct loadData : public marketData {
 public:
  loadData();
  ~loadData(){};
  void LoadDepthMarketDataToCsv(CThostFtdcDepthMarketDataField *pD);
  void FormDepthMarketData2Stringflow(CThostFtdcDepthMarketDataField *pD);

  void LoadDepthMarketDataToCsv(XTPMD *pD);
  void FormDepthMarketData2Stringflow(XTPMD *pD);

  bool ClassifyContractFiles(void);
  bool MoveContractToFolder(std::string contractName, std::string exchangeName);

 private:
  std::string history_tick_folder;
  char dataflow[600];
  char titleflow[400] = {
      "InstrumentID,TradingDay,UpdateTime,LastPrice,BidPrice1,BidVolume1,"
      "AskPrice1,AskVolume1,BidPrice2,BidVolume2,AskPrice2,AskVolume2,"
      "BidPrice3,BidVolume3,AskPrice3,AskVolume3,BidPrice4,BidVolume4,"
      "AskPrice4,AskVolume4,BidPrice5,BidVolume5,AskPrice5,AskVolume5,Volume,"
      "Turnover,OpenInterest,UpperLimitPrice,LowerLimitPrice,OpenPrice,"
      "PreSettlementPrice,PreClosePrice,PreOpenInterest,SettlementPrice"};
};

#endif
