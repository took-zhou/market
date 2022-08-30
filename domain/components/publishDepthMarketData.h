#ifndef PUBLISH_DEPTH_MARKET_DATA_H
#define PUBLISH_DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/extern/xtp/inc/xtp_quote_api.h"
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

  // 超时发送默认数据
  void heartbeatDetect();
  void once_from_default(const publishControl &pc, const std::string &keyname);

  // ctp深度行情发送
 public:
  void directForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD);
  void once_from_dataflow(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);

 private:
  void once_from_dataflow_select_rawtick(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);
  void once_from_dataflow_select_level1(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);
  bool isValidLevel1Data(const publishControl &pc, CThostFtdcDepthMarketDataField *pD);

  // xtp深度行情发送
 public:
  void directForwardDataToStrategy(XTPMD *pD);
  void once_from_dataflow(const publishControl &pc, XTPMD *pD);

 private:
  void once_from_dataflow_select_rawtick(const publishControl &pc, XTPMD *pD);
  void once_from_dataflow_select_level1(const publishControl &pc, XTPMD *pD);
  bool isValidLevel1Data(const publishControl &pc, XTPMD *pD);

 private:
  const U8 data_level = 1;
};

#endif