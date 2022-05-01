#ifndef PUBLISH_DEPTH_MARKET_DATA_H
#define PUBLISH_DEPTH_MARKET_DATA_H

#include <vector>
#include <string>
#include <map>

#include "common/self/dci/Role.h"
#include "common/self/basetype.h"
#include "common/extern/ctp/inc/ThostFtdcUserApiStruct.h"
#include "common/self/protobuf/market-strategy.pb.h"
#include "common/self/utils.h"

#include "market/domain/components/depthMarketData.h"
#include "market/domain/components/controlPara.h"

#define MARKET_BUF_SIZE     100u

struct deepTickData
{
    pthread_mutex_t sm_mutex;
    CThostFtdcDepthMarketDataField datafield[MARKET_BUF_SIZE];
};

struct publishData :public marketData{
public:
    publishData();
    ~publishData() {};

    // 直接传输到策略端
    void directForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD);
    void once_from_dataflow(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD);
    // 先暂存共享内存，再传输给策略端
    void publishToStrategy(const std::string keyname);
    void once_from_datafield(std::map<std::string, publishControl>::iterator pc);
    void insertDataToTickDataPool(CThostFtdcDepthMarketDataField *pD);

    deepTickData *tickData;
private:
    void once_from_dataflow_select_rawtick(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD);
    void once_from_dataflow_select_level1(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD);
    bool isValidLevel1Data(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD);
};

#endif