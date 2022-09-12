#ifndef PUBLISH_DEPTH_MARKET_DATA_H
#define PUBLISH_DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/extern/xtp/inc/xtp_quote_api.h"

#include "common/self/dci/role.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

#include "market/domain/components/control_para.h"
#include "market/domain/components/depth_market_data.h"

struct PublishData : public MarketData {
 public:
  PublishData();
  ~PublishData(){};

  // 超时发送默认数据
  void HeartBeatDetect();
  void OnceFromDefault(const PublishControl &pc, const std::string &keyname);

  // ctp深度行情发送
 public:
  void DirectForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD);
  void OnceFromDataflow(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD);

 private:
  void OnceFromDataflowSelectRawtick(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD);
  void OnceFromDataflowSelectLevel1(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD);
  bool IsValidLevel1Data(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD);

  // xtp深度行情发送
 public:
  void DirectForwardDataToStrategy(XTPMD *pD);
  void OnceFromDataflow(const PublishControl &pc, XTPMD *pD);

 private:
  void OnceFromDataflowSelectRawtick(const PublishControl &pc, XTPMD *pD);
  void OnceFromDataflowSelectLevel1(const PublishControl &pc, XTPMD *pD);
  bool IsValidLevel1Data(const PublishControl &pc, XTPMD *pD);

 private:
  const uint8_t kDataLevel = 1;
  const uint32_t kHeartBeatWaitTime = 60;
};

#endif