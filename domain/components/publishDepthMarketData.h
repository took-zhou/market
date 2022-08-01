#ifndef PUBLISH_DEPTH_MARKET_DATA_H
#define PUBLISH_DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/self/basetype.h"
#include "common/self/dci/Role.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

#include "market/domain/components/controlPara.h"
#include "market/domain/components/depthMarketData.h"

struct publishData : public marketData {
 public:
  publishData();
  ~publishData(){};

  // 直接传输到策略端
  void directForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD);
  void once_from_dataflow(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);

  // 超时发送默认数据
  void heartbeatDetect();
  void once_from_default(const publishControl &pc, const string &keyname);

 private:
  void once_from_dataflow_select_rawtick(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);
  void once_from_dataflow_select_level1(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);
  bool isValidLevel1Data(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);

 private:
  const U8 data_level = 1;
};

#endif