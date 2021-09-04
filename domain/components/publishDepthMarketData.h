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

#define MARKET_BUF_SIZE     100u

struct tickDataPool
{
    utils::InstrumtntID id;
    mutable U32 index;
};

struct tickDataPoolSortCriterion
{
public:
    bool operator() (const tickDataPool &a, const tickDataPool &b) const 
    {
        // 只以合约名作为排序依据
        if (a.id.ins < b.id.ins)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

struct deepTickData
{
    pthread_mutex_t sm_mutex;
    CThostFtdcDepthMarketDataField datafield[MARKET_BUF_SIZE];
};

struct publishControl
{
    std::set<tickDataPool, tickDataPoolSortCriterion> instrumentList;
    std::set<std::string> keywordList;
    market_strategy::TickStartStopIndication_MessageType indication = market_strategy::TickStartStopIndication_MessageType_reserve;
    // 单位us
    U32 interval = 0;
    bool directforward = false;
    int thread_uniqueness_cnt = 0;
};

struct publishData :public marketData{
public:
    publishData();
    ~publishData() {};

    std::vector<utils::InstrumtntID> getInstrumentList(void);

    // 直接传输到策略端
    void directForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD);
    void once_from_dataflow(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD);
    // 先暂存共享内存，再传输给策略端
    void publishToStrategy(const std::string keyname);
    void once_from_datafield(std::map<std::string, publishControl>::iterator pc);
    void insertDataToTickDataPool(CThostFtdcDepthMarketDataField *pD);

    deepTickData *tickData;
    void buildKeywordList(const std::string keyname, std::vector<std::string> &keyword);
    void buildInstrumentList(const std::string keyname, std::vector<utils::InstrumtntID> const &nameVec);
    void setStartStopIndication(const std::string keyname, market_strategy::TickStartStopIndication_MessageType _indication);
    void setInterval(const std::string keyname, float _interval);
    void setDirectForwardingFlag(const std::string keyname, bool flag);

    void updatePublishInstrumentInfo(void);
private:
    std::set<tickDataPool, tickDataPoolSortCriterion> instrumentList;
    std::map<std::string, publishControl> publishCtrlMap;
};

#endif