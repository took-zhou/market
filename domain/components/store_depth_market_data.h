#ifndef STORE_DEPTH_MARKET_DATA_H
#define STORE_DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/extern/otp/inc/mds_api/mds_async_api.h"
#include "common/extern/xtp/inc/xquote_api_struct.h"
#include "market/domain/components/depth_market_data.h"

struct LoadData : public MarketData {
 public:
  LoadData();
  ~LoadData(){};
  void FormDepthMarketData2Stringflow(CThostFtdcDepthMarketDataField *p_d);
  void LoadDepthMarketDataToCsv(CThostFtdcDepthMarketDataField *p_d);

  void FormDepthMarketData2Stringflow(XTPMD *p_d);
  void LoadDepthMarketDataToCsv(XTPMD *p_d);

  void FormDepthMarketData2Stringflow(MdsMktDataSnapshotT *p_d);
  void LoadDepthMarketDataToCsv(MdsMktDataSnapshotT *p_d);

  bool ClassifyContractFiles(void);
  bool MoveContractToFolder(const std::string &contract_name, const std::string &exchange_name);

 private:
  std::string history_tick_folder_;
  char dataflow_[600];
  char titleflow_[400] = {
      "InstrumentID,TradingDay,UpdateTime,LastPrice,BidPrice1,BidVolume1,"
      "AskPrice1,AskVolume1,BidPrice2,BidVolume2,AskPrice2,AskVolume2,"
      "BidPrice3,BidVolume3,AskPrice3,AskVolume3,BidPrice4,BidVolume4,"
      "AskPrice4,AskVolume4,BidPrice5,BidVolume5,AskPrice5,AskVolume5,Volume,"
      "Turnover,OpenInterest,UpperLimitPrice,LowerLimitPrice,OpenPrice,"
      "PreSettlementPrice,PreClosePrice,PreOpenInterest,SettlementPrice"};
};

#endif
