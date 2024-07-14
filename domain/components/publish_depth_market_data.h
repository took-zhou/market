#ifndef PUBLISH_DEPTH_MARKET_DATA_H
#define PUBLISH_DEPTH_MARKET_DATA_H

#include <map>
#include <string>
#include <vector>

#include "common/extern//btp/inc/btp_market_struct.h"
#include "common/extern//ftp/inc/ftp_market_struct.h"
#include "common/extern//gtp/inc/gtp_market_struct.h"
#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"

#include "common/self/protobuf/strategy-market.pb.h"
#include "market/domain/components/depth_market_data.h"

struct PublishData : public MarketData {
 public:
  PublishData();
  ~PublishData(){};

  // 超时发送默认数据
  void HeartBeatDetect();
  void UpdateLoginHeartBeat();
  void UpdateLogoutHeartBeat();

  void OnceFromDefault(const std::string &ins);
  void OnceFromDefault(FtpMarketDataStruct *p_d);

  // ctp深度行情发送
  void DirectForwardDataToStrategy(CThostFtdcDepthMarketDataField *p_d);
  void OnceFromDataflow(CThostFtdcDepthMarketDataField *p_d);

  // xtp深度行情发送
  void DirectForwardDataToStrategy(XTPMD *p_d);
  void OnceFromDataflow(XTPMD *p_d);

  // btp深度行情发送
  void DirectForwardDataToStrategy(BtpMarketDataStruct *p_d);
  void OnceFromDataflow(BtpMarketDataStruct *p_d);

  // otp深度行情发送
  void DirectForwardDataToStrategy(MdsMktDataSnapshotT *p_d);
  void OnceFromDataflow(MdsMktDataSnapshotT *p_d);

  // ftp深度行情发送
  void DirectForwardDataToStrategy(FtpMarketDataStruct *p_d);
  void OnceFromDataflow(FtpMarketDataStruct *p_d);

  // gtp深度行情发送
  void DirectForwardDataToStrategy(GtpMarketDataStruct *p_d);
  void OnceFromDataflow(GtpMarketDataStruct *p_d);

 private:
  const uint8_t kDataLevel_ = 1;
  const uint32_t kHeartBeatWaitTime_ = 60;
};

#endif