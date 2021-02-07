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
    int index;
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

struct publishData :public marketData{
public:
    publishData();
    ~publishData() {};
    void buildKeywordList(std::vector<std::string> &keyword);
    void buildInstrumentList(std::vector<utils::InstrumtntID> const &nameVec);

    std::vector<std::string> getKeywordList(void);
    std::vector<utils::InstrumtntID> getInstrumentList(void);

    void insertDataToTickDataPool(CThostFtdcDepthMarketDataField *pD);
    // 传输给策略端
    void publishToStrategy(void);
    void once(void);

    deepTickData *tickData;
    void setStartStopIndication(market_strategy::TickStartStopIndication_MessageType _indication);
    void setInterval(int _interval);

private:
    std::set<std::string> keywordList;
    std::set<tickDataPool, tickDataPoolSortCriterion> instrumentList;
    market_strategy::TickStartStopIndication_MessageType indication = market_strategy::TickStartStopIndication_MessageType_reserve;
    int interval = 0;
    int thread_uniqueness_cnt = 0;
};

#endif