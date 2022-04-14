#ifndef CONTROL_PARA_H
#define CONTROL_PARA_H

#include <vector>
#include <string>
#include <map>

#include "common/self/utils.h"
#include "common/self/basetype.h"
#include "common/self/protobuf/market-strategy.pb.h"

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

struct publishControl
{
    std::set<tickDataPool, tickDataPoolSortCriterion> instrumentList;
    market_strategy::TickStartStopIndication_MessageType indication = market_strategy::TickStartStopIndication_MessageType_reserve;
    // 单位us
    U32 interval = 0;
    bool directforward = false;
    std::string source = "rawtick"; // 默认 rawtick
    int thread_uniqueness_cnt = 0;
};

struct controlPara
{
    controlPara();
    ~controlPara() {};

    bool load_from_json(void);
    bool write_to_json(void);

    std::vector<utils::InstrumtntID> getInstrumentList(void);
    std::vector<std::string> getKeyNameList(void);

    void buildInstrumentList(const std::string keyname, std::vector<utils::InstrumtntID> const &nameVec);
    void setStartStopIndication(const std::string keyname, market_strategy::TickStartStopIndication_MessageType _indication);
    void setInterval(const std::string keyname, float _interval);
    void setDirectForwardingFlag(const std::string keyname, bool flag);
    void setSource(const std::string keyname, std::string _source);

    void updatePublishInstrumentInfo(void);

    std::set<tickDataPool, tickDataPoolSortCriterion> instrumentList;
    std::map<std::string, publishControl> publishCtrlMap;

private:
    std::string json_path = "";
};

#endif
