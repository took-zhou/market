// 实时时间获取
#include <stddef.h>
#include <time.h>

// 文件夹及文件操作相关
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <dirent.h>

// 共享内存
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

//自定义头文件
#include "market/domain/components/publishDepthMarketData.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-strategy.pb.h"
#include "market/infra/recer/ctpRecer.h"
#include "market/infra/recerSender.h"
#include "market/domain/marketService.h"

publishData::publishData()
{
    int shm_id;
    key_t key;
    pthread_mutexattr_t attr;

    //Write once, read multiple times, mutex before read, mutex between read and write
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    //Create a Shared memory area
    auto& jsonCfg = utils::JsonConfig::getInstance();
    const std::string sharepath = jsonCfg.getConfig("market","ShareMemoryAddr").get<std::string>();
    string command = "touch " + sharepath;
    if(access(sharepath.c_str(), F_OK) == -1)
    {
        system(command.c_str());
    }
    if(( key = ftok(sharepath.c_str(), 'z')) < 0)
    {
        ERROR_LOG("ftok error.");
        exit(1);
    }

    if( (shm_id = shmget(key,sizeof(deepTickData),IPC_CREAT|0666)) == -1 )
    {
        ERROR_LOG("create shared memory error.");
        exit(1);
    }

    tickData = (deepTickData*)shmat(shm_id, NULL, 0);
    if(  (long)tickData == -1)
    {
        ERROR_LOG("attach shared memory error.");
        exit(1);
    }

    //Initializes the mutex
    pthread_mutex_init(&tickData->sm_mutex, &attr);

    INFO_LOG("init share message address ok.");
}

void publishData::once_from_datafield(std::map<std::string, publishControl>::iterator pc)
{
    char timeArray[100] = {0};
    market_strategy::message tick;
    auto tick_data = tick.mutable_tick_data();

    getLocalTime(timeArray);
    tick_data->set_time_point(timeArray);

    int ik = pthread_mutex_lock(&(tickData->sm_mutex));

    auto instrument_iter = pc->second.instrumentList.begin();
    while (instrument_iter != pc->second.instrumentList.end())
    {
        if (instrument_iter->id.ins == string(tickData->datafield[instrument_iter->index].InstrumentID))
        {
            auto iter = tick_data->add_tick_list();
            iter->set_state(market_strategy::TickData_TickState_active);
            iter->set_instrument_id(tickData->datafield[instrument_iter->index].InstrumentID);

            if (pc->second.keywordList.find("LastPrice") != end(pc->second.keywordList))
            {
                auto lastPrice = iter->mutable_last_price();
                lastPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].LastPrice)));
            }
            if (pc->second.keywordList.find("BidPrice1") != end(pc->second.keywordList))
            {
                auto bidPrice1 = iter->mutable_bid_price1();
                bidPrice1->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].BidPrice1)));
            }
            if (pc->second.keywordList.find("BidVolume1") != end(pc->second.keywordList))
            {
                auto bidVolume1 = iter->mutable_bid_volume1();
                bidVolume1->set_value(tickData->datafield[instrument_iter->index].BidVolume1);
            }
            if (pc->second.keywordList.find("AskPrice1") != end(pc->second.keywordList))
            {
                auto askPrice1 = iter->mutable_ask_price1();
                askPrice1->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].AskPrice1)));
            }
            if (pc->second.keywordList.find("AskVolume1") != end(pc->second.keywordList))
            {
                auto askVolume1 = iter->mutable_ask_volume1();
                askVolume1->set_value(tickData->datafield[instrument_iter->index].AskVolume1);
            }
            if (pc->second.keywordList.find("Turnover") != end(pc->second.keywordList))
            {
                auto turnOver = iter->mutable_turnover();
                turnOver->set_value(tickData->datafield[instrument_iter->index].Turnover);
            }
            if (pc->second.keywordList.find("OpenInterest") != end(pc->second.keywordList))
            {
                auto openInterest = iter->mutable_open_interest();
                openInterest->set_value(tickData->datafield[instrument_iter->index].OpenInterest);
            }
            if (pc->second.keywordList.find("UpperLimitPrice") != end(pc->second.keywordList))
            {
                auto upperLimitPrice = iter->mutable_upper_limit_price();
                upperLimitPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].UpperLimitPrice)));
            }
            if (pc->second.keywordList.find("LowerLimitPrice") != end(pc->second.keywordList))
            {
                auto lowerLimitPrice = iter->mutable_lower_limit_price();
                lowerLimitPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].LowerLimitPrice)));
            }
            if (pc->second.keywordList.find("OpenPrice") != end(pc->second.keywordList))
            {
                auto openPrice = iter->mutable_open_price();
                openPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].OpenPrice)));
            }
            if (pc->second.keywordList.find("PreSettlementPrice") != end(pc->second.keywordList))
            {
                auto preSettleMentPrice = iter->mutable_pre_settlement_price();
                preSettleMentPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreSettlementPrice)));
            }
            if (pc->second.keywordList.find("PreClosePrice") != end(pc->second.keywordList))
            {
                auto preClosePrice = iter->mutable_pre_close_price();
                preClosePrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreClosePrice)));
            }
            if (pc->second.keywordList.find("PreOpenInterest") != end(pc->second.keywordList))
            {
                auto preOpenInterest = iter->mutable_pre_open_interest();
                preOpenInterest->set_value(tickData->datafield[instrument_iter->index].PreOpenInterest);
            }
            if (pc->second.keywordList.find("Volume") != end(pc->second.keywordList))
            {
                auto volume = iter->mutable_volume();
                volume->set_value(tickData->datafield[instrument_iter->index].Volume);
            }
        }

        instrument_iter++;
    }
    ik = pthread_mutex_unlock(&(tickData->sm_mutex));
    std::string tickStr;
    tick.SerializeToString(&tickStr);

    auto& recerSender = RecerSender::getInstance();
    char temp_topic[100];
    sprintf(temp_topic, "market_strategy.TickData.%s", pc->first.c_str());
    recerSender.ROLE(Sender).ROLE(ProxySender).send(temp_topic, tickStr.c_str());
}

void publishData::once_from_dataflow(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD)
{
    char timeArray[100] = {0};
    market_strategy::message tick;
    auto tick_data = tick.mutable_tick_data();

    getLocalTime(timeArray);
    tick_data->set_time_point(timeArray);

    auto iter = tick_data->add_tick_list();
    iter->set_state(market_strategy::TickData_TickState_active);
    iter->set_instrument_id(pD->InstrumentID);

    if (pc->second.keywordList.find("LastPrice") != end(pc->second.keywordList))
    {
        auto lastPrice = iter->mutable_last_price();
        lastPrice->set_value(std::to_string(max2zero(pD->LastPrice)));
    }
    if (pc->second.keywordList.find("BidPrice1") != end(pc->second.keywordList))
    {
        auto bidPrice1 = iter->mutable_bid_price1();
        bidPrice1->set_value(std::to_string(max2zero(pD->BidPrice1)));
    }
    if (pc->second.keywordList.find("BidVolume1") != end(pc->second.keywordList))
    {
        auto bidVolume1 = iter->mutable_bid_volume1();
        bidVolume1->set_value(pD->BidVolume1);
    }
    if (pc->second.keywordList.find("AskPrice1") != end(pc->second.keywordList))
    {
        auto askPrice1 = iter->mutable_ask_price1();
        askPrice1->set_value(std::to_string(max2zero(pD->AskPrice1)));
    }
    if (pc->second.keywordList.find("AskVolume1") != end(pc->second.keywordList))
    {
        auto askVolume1 = iter->mutable_ask_volume1();
        askVolume1->set_value(pD->AskVolume1);
    }
    if (pc->second.keywordList.find("Turnover") != end(pc->second.keywordList))
    {
        auto turnOver = iter->mutable_turnover();
        turnOver->set_value(pD->Turnover);
    }
    if (pc->second.keywordList.find("OpenInterest") != end(pc->second.keywordList))
    {
        auto openInterest = iter->mutable_open_interest();
        openInterest->set_value(pD->OpenInterest);
    }
    if (pc->second.keywordList.find("UpperLimitPrice") != end(pc->second.keywordList))
    {
        auto upperLimitPrice = iter->mutable_upper_limit_price();
        upperLimitPrice->set_value(std::to_string(max2zero(pD->UpperLimitPrice)));
    }
    if (pc->second.keywordList.find("LowerLimitPrice") != end(pc->second.keywordList))
    {
        auto lowerLimitPrice = iter->mutable_lower_limit_price();
        lowerLimitPrice->set_value(std::to_string(max2zero(pD->LowerLimitPrice)));
    }
    if (pc->second.keywordList.find("OpenPrice") != end(pc->second.keywordList))
    {
        auto openPrice = iter->mutable_open_price();
        openPrice->set_value(std::to_string(max2zero(pD->OpenPrice)));
    }
    if (pc->second.keywordList.find("PreSettlementPrice") != end(pc->second.keywordList))
    {
        auto preSettleMentPrice = iter->mutable_pre_settlement_price();
        preSettleMentPrice->set_value(std::to_string(max2zero(pD->PreSettlementPrice)));
    }
    if (pc->second.keywordList.find("PreClosePrice") != end(pc->second.keywordList))
    {
        auto preClosePrice = iter->mutable_pre_close_price();
        preClosePrice->set_value(std::to_string(max2zero(pD->PreClosePrice)));
    }
    if (pc->second.keywordList.find("PreOpenInterest") != end(pc->second.keywordList))
    {
        auto preOpenInterest = iter->mutable_pre_open_interest();
        preOpenInterest->set_value(pD->PreOpenInterest);
    }
    if (pc->second.keywordList.find("Volume") != end(pc->second.keywordList))
    {
        auto volume = iter->mutable_volume();
        volume->set_value(pD->Volume);
    }

    std::string tickStr;
    tick.SerializeToString(&tickStr);

    auto& recerSender = RecerSender::getInstance();
    char temp_topic[200];
    sprintf(temp_topic, "market_strategy.TickData.%s", pD->InstrumentID);
    recerSender.ROLE(Sender).ROLE(ProxySender).send(temp_topic, tickStr.c_str());
}

void publishData::publishToStrategy(const string keyname)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end() && iter->second.thread_uniqueness_cnt++ == 0)
    {
        INFO_LOG("publishDataFuc prepare ok");
        while (1)
        {
            if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_start)
            {
                auto& marketSer = MarketService::getInstance();
                if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE)
                {
                    once_from_datafield(iter);
                }
            }
            else if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_stop)
            {
                INFO_LOG("publishToStrategy is stopping.");
            }
            else if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_finish)
            {
                INFO_LOG("is going to exit publishToStrategy thread.");
                publishCtrlMap.erase(iter);
                break;
            }
            usleep(iter->second.interval);
        }
    }
}

void publishData::directForwardDataToStrategy(CThostFtdcDepthMarketDataField * pD)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(pD->InstrumentID);
    if (iter != publishCtrlMap.end() && iter->second.directforward == true)
    {
        if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_start)
        {
            auto& marketSer = MarketService::getInstance();
            if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE)
            {
                once_from_dataflow(iter, pD);
            }
        }
        else if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_stop)
        {
            INFO_LOG("publishToStrategy is stopping.");
        }
        else if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_finish)
        {
            INFO_LOG("is going to exit publishToStrategy thread.");
            publishCtrlMap.erase(iter);
        }
    }
}

void publishData::insertDataToTickDataPool(CThostFtdcDepthMarketDataField * pD)
{
    int ik = pthread_mutex_lock(&(tickData->sm_mutex));
    tickDataPool tempData;
    tempData.id.ins = string(pD->InstrumentID);

    auto iter = instrumentList.find(tempData);
    if (iter != instrumentList.end())
    {
        if (isValidTickData(pD) == true)
        {
            memcpy(&(tickData->datafield[iter->index]), pD, sizeof(CThostFtdcDepthMarketDataField));
        }
    }
    ik = pthread_mutex_unlock(&(tickData->sm_mutex));
}

void publishData::setInterval(const std::string keyname, float _interval)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.interval = (U32)(_interval*1000000);
    }
}

void publishData::setDirectForwardingFlag(const std::string keyname, bool flag)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.directforward = flag;
    }
}

void publishData::setStartStopIndication(const std::string keyname, market_strategy::TickStartStopIndication_MessageType _indication)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.indication = _indication;
    }
    else
    {
        ERROR_LOG("Please first send tickdatareq action, keyname: %s.", keyname.c_str());
    }
}

void publishData::buildKeywordList(const string keyname, std::vector<std::string> &keyword)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.keywordList.clear();
        for (int i = 0; i < keyword.size(); i++)
        {
            iter->second.keywordList.insert(keyword[i]);
        }
    }
}

void publishData::buildInstrumentList(const string keyname, std::vector<utils::InstrumtntID> const &nameVec)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter == publishCtrlMap.end())
    {
        publishControl tempControl;
        for (int i = 0; i < nameVec.size(); i++)
        {
            tickDataPool tempData;
            tempData.id = nameVec[i];
            auto pos = instrumentList.find(tempData);
            if (pos == end(instrumentList))
            {
                tempData.index = instrumentList.size();
                instrumentList.insert(tempData);
            }
            else
            {
                tempData.index = pos->index;
            }
            tempControl.instrumentList.insert(tempData);
        }

        INFO_LOG("insert keyname: %s", keyname.c_str());
        publishCtrlMap.insert(make_pair(keyname, tempControl));
    }
}

std::vector<utils::InstrumtntID> publishData::getInstrumentList(void)
{
    std::vector<utils::InstrumtntID> instrument_vector;
    instrument_vector.clear();
    auto iter = instrumentList.begin();
    while (iter != instrumentList.end())
    {
        instrument_vector.push_back(iter->id);
        iter++;
    }

    return instrument_vector;
}

std::vector<std::vector<utils::InstrumtntID>> publishData::getPublishList(void)
{
    std::vector<std::vector<utils::InstrumtntID>> temp_vec;
    temp_vec.clear();
    std::map<string, publishControl>::iterator iter;
    
    for (iter = publishCtrlMap.begin(); iter != publishCtrlMap.end(); iter++)
    {
        std::vector<utils::InstrumtntID> instrument_vector;
        instrument_vector.clear();
        auto iter2 = iter->second.instrumentList.begin();
        while (iter2 != iter->second.instrumentList.end())
        {
            instrument_vector.push_back(iter2->id);
        }
        temp_vec.push_back(instrument_vector);
    }

    return temp_vec;
}

void publishData::updatePublishInstrumentInfo(void)
{
    instrumentList.clear();
    std::map<string, publishControl>::iterator iter;
    U32 index_count = 0;
    for (iter = publishCtrlMap.begin(); iter != publishCtrlMap.end(); iter++)
    {

        auto iter2 = iter->second.instrumentList.begin();
        while (iter2 != iter->second.instrumentList.end())
        {
            iter2->index = index_count;
            instrumentList.insert(*iter2);

            index_count++;
            iter2++;
        }
    }
}