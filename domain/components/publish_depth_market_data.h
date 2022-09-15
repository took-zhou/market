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
  void OnceFromDefault(const PublishControl &p_c, const std::string &keyname);

  // ctp深度行情发送
 public:
  void DirectForwardDataToStrategy(CThostFtdcDepthMarketDataField *p_d);
  void OnceFromDataflow(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d);

 private:
  void OnceFromDataflowSelectRawtick(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d);
  void OnceFromDataflowSelectLevel1(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d);
  bool IsValidLevel1Data(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d);

  // xtp深度行情发送
 public:
  void DirectForwardDataToStrategy(XTPMD *p_d);
  void OnceFromDataflow(const PublishControl &p_c, XTPMD *p_d);

 private:
  void OnceFromDataflowSelectRawtick(const PublishControl &p_c, XTPMD *p_d);
  void OnceFromDataflowSelectLevel1(const PublishControl &p_c, XTPMD *p_d);
  bool IsValidLevel1Data(const PublishControl &p_c, XTPMD *p_d);

 private:
  const uint8_t kDataLevel_ = 1;
  const uint32_t kHeartBeatWaitTime_ = 60;
};

#endif