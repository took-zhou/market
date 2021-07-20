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

void publishData::once_from_datafield(void)
{
    char timeArray[100] = {0};
    market_strategy::message tick;
    auto tick_data = tick.mutable_tick_data();

    getLocalTime(timeArray);
    tick_data->set_time_point(timeArray);

    int ik = pthread_mutex_lock(&(tickData->sm_mutex));

    auto instrument_iter = instrumentList.begin();
    while (instrument_iter != instrumentList.end())
    {
        if (instrument_iter->id.ins == string(tickData->datafield[instrument_iter->index].InstrumentID))
        {
            auto iter = tick_data->add_tick_list();
            iter->set_state(market_strategy::TickData_TickState_active);

            string temp_ins = tickData->datafield[instrument_iter->index].InstrumentID;
            transform(temp_ins.begin(), temp_ins.end(), temp_ins.begin(), ::tolower);
            iter->set_instrument_id(temp_ins);

            if (keywordList.find("LastPrice") != end(keywordList))
            {
                auto lastPrice = iter->mutable_last_price();
                lastPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].LastPrice)));
            }
            if (keywordList.find("BidPrice1") != end(keywordList))
            {
                auto bidPrice1 = iter->mutable_bid_price1();
                bidPrice1->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].BidPrice1)));
            }
            if (keywordList.find("BidVolume1") != end(keywordList))
            {
                auto bidVolume1 = iter->mutable_bid_volume1();
                bidVolume1->set_value(tickData->datafield[instrument_iter->index].BidVolume1);
            }
            if (keywordList.find("AskPrice1") != end(keywordList))
            {
                auto askPrice1 = iter->mutable_ask_price1();
                askPrice1->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].AskPrice1)));
            }
            if (keywordList.find("AskVolume1") != end(keywordList))
            {
                auto askVolume1 = iter->mutable_ask_volume1();
                askVolume1->set_value(tickData->datafield[instrument_iter->index].AskVolume1);
            }
            if (keywordList.find("Turnover") != end(keywordList))
            {
                auto turnOver = iter->mutable_turnover();
                turnOver->set_value(tickData->datafield[instrument_iter->index].Turnover);
            }
            if (keywordList.find("OpenInterest") != end(keywordList))
            {
                auto openInterest = iter->mutable_open_interest();
                openInterest->set_value(tickData->datafield[instrument_iter->index].OpenInterest);
            }
            if (keywordList.find("UpperLimitPrice") != end(keywordList))
            {
                auto upperLimitPrice = iter->mutable_upper_limit_price();
                upperLimitPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].UpperLimitPrice)));
            }
            if (keywordList.find("LowerLimitPrice") != end(keywordList))
            {
                auto lowerLimitPrice = iter->mutable_lower_limit_price();
                lowerLimitPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].LowerLimitPrice)));
            }
            if (keywordList.find("OpenPrice") != end(keywordList))
            {
                auto openPrice = iter->mutable_open_price();
                openPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].OpenPrice)));
            }
            if (keywordList.find("PreSettlementPrice") != end(keywordList))
            {
                auto preSettleMentPrice = iter->mutable_pre_settlement_price();
                preSettleMentPrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreSettlementPrice)));
            }
            if (keywordList.find("PreClosePrice") != end(keywordList))
            {
                auto preClosePrice = iter->mutable_pre_close_price();
                preClosePrice->set_value(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreClosePrice)));
            }
            if (keywordList.find("PreOpenInterest") != end(keywordList))
            {
                auto preOpenInterest = iter->mutable_pre_open_interest();
                preOpenInterest->set_value(tickData->datafield[instrument_iter->index].PreOpenInterest);
            }
            if (keywordList.find("Volume") != end(keywordList))
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
    recerSender.ROLE(Sender).ROLE(ProxySender).send("market_strategy.TickData", tickStr.c_str());
}

void publishData::once_from_dataflow(CThostFtdcDepthMarketDataField *pD)
{
    char timeArray[100] = {0};
    market_strategy::message tick;
    auto tick_data = tick.mutable_tick_data();

    getLocalTime(timeArray);
    tick_data->set_time_point(timeArray);

    auto iter = tick_data->add_tick_list();
    iter->set_state(market_strategy::TickData_TickState_active);

    string temp_ins = pD->InstrumentID;
    transform(temp_ins.begin(), temp_ins.end(), temp_ins.begin(), ::tolower);
    iter->set_instrument_id(temp_ins);

    if (keywordList.find("LastPrice") != end(keywordList))
    {
        auto lastPrice = iter->mutable_last_price();
        lastPrice->set_value(std::to_string(max2zero(pD->LastPrice)));
    }
    if (keywordList.find("BidPrice1") != end(keywordList))
    {
        auto bidPrice1 = iter->mutable_bid_price1();
        bidPrice1->set_value(std::to_string(max2zero(pD->BidPrice1)));
    }
    if (keywordList.find("BidVolume1") != end(keywordList))
    {
        auto bidVolume1 = iter->mutable_bid_volume1();
        bidVolume1->set_value(pD->BidVolume1);
    }
    if (keywordList.find("AskPrice1") != end(keywordList))
    {
        auto askPrice1 = iter->mutable_ask_price1();
        askPrice1->set_value(std::to_string(max2zero(pD->AskPrice1)));
    }
    if (keywordList.find("AskVolume1") != end(keywordList))
    {
        auto askVolume1 = iter->mutable_ask_volume1();
        askVolume1->set_value(pD->AskVolume1);
    }
    if (keywordList.find("Turnover") != end(keywordList))
    {
        auto turnOver = iter->mutable_turnover();
        turnOver->set_value(pD->Turnover);
    }
    if (keywordList.find("OpenInterest") != end(keywordList))
    {
        auto openInterest = iter->mutable_open_interest();
        openInterest->set_value(pD->OpenInterest);
    }
    if (keywordList.find("UpperLimitPrice") != end(keywordList))
    {
        auto upperLimitPrice = iter->mutable_upper_limit_price();
        upperLimitPrice->set_value(std::to_string(max2zero(pD->UpperLimitPrice)));
    }
    if (keywordList.find("LowerLimitPrice") != end(keywordList))
    {
        auto lowerLimitPrice = iter->mutable_lower_limit_price();
        lowerLimitPrice->set_value(std::to_string(max2zero(pD->LowerLimitPrice)));
    }
    if (keywordList.find("OpenPrice") != end(keywordList))
    {
        auto openPrice = iter->mutable_open_price();
        openPrice->set_value(std::to_string(max2zero(pD->OpenPrice)));
    }
    if (keywordList.find("PreSettlementPrice") != end(keywordList))
    {
        auto preSettleMentPrice = iter->mutable_pre_settlement_price();
        preSettleMentPrice->set_value(std::to_string(max2zero(pD->PreSettlementPrice)));
    }
    if (keywordList.find("PreClosePrice") != end(keywordList))
    {
        auto preClosePrice = iter->mutable_pre_close_price();
        preClosePrice->set_value(std::to_string(max2zero(pD->PreClosePrice)));
    }
    if (keywordList.find("PreOpenInterest") != end(keywordList))
    {
        auto preOpenInterest = iter->mutable_pre_open_interest();
        preOpenInterest->set_value(pD->PreOpenInterest);
    }
    if (keywordList.find("Volume") != end(keywordList))
    {
        auto volume = iter->mutable_volume();
        volume->set_value(pD->Volume);
    }

    std::string tickStr;
    tick.SerializeToString(&tickStr);

    auto& recerSender = RecerSender::getInstance();
    recerSender.ROLE(Sender).ROLE(ProxySender).send("market_strategy.TickData", tickStr.c_str());
}

void publishData::publishToStrategy(void)
{
    if (thread_uniqueness_cnt++ == 0)
    {
        INFO_LOG("publishDataFuc prepare ok");
        while (1)
        {
            if (indication == market_strategy::TickStartStopIndication_MessageType_start)
            {
                auto& marketSer = MarketService::getInstance();
                if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE)
                {
                    once_from_datafield();
                }
            }
            else if (indication == market_strategy::TickStartStopIndication_MessageType_stop)
            {
                INFO_LOG("publishToStrategy is stopping.");
            }
            else if (indication == market_strategy::TickStartStopIndication_MessageType_finish)
            {
                INFO_LOG("is going to exit publishToStrategy thread.");
                indication = market_strategy::TickStartStopIndication_MessageType_reserve;
                keywordList.clear();
                instrumentList.clear();
                thread_uniqueness_cnt = 0;
                break;
            }
            usleep(interval);
        }
    }
}

void publishData::directForwardDataToStrategy(CThostFtdcDepthMarketDataField * pD)
{
    if (indication == market_strategy::TickStartStopIndication_MessageType_start)
    {
        auto& marketSer = MarketService::getInstance();
        if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE)
        {
            once_from_dataflow(pD);
        }
    }
    else if (indication == market_strategy::TickStartStopIndication_MessageType_stop)
    {
        INFO_LOG("publishToStrategy is stopping.");
    }
    else if (indication == market_strategy::TickStartStopIndication_MessageType_finish)
    {
        INFO_LOG("is going to exit publishToStrategy thread.");
        indication = market_strategy::TickStartStopIndication_MessageType_reserve;
        directforward = false;
        keywordList.clear();
        instrumentList.clear();
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

void publishData::setInterval(float _interval)
{
    interval = (U32)(_interval*1000000);
}

void publishData::setDirectForwardingFlag(bool flag)
{
    directforward = flag;
}

bool publishData::isDirectForwarding(void)
{
    return directforward;
}

void publishData::setStartStopIndication(market_strategy::TickStartStopIndication_MessageType _indication)
{
    indication = _indication;
}

void publishData::buildKeywordList(std::vector<std::string> &keyword)
{
    for (int i = 0; i < keyword.size(); i++)
    {
        if (keywordList.find(keyword[i]) == end(keywordList))
        {
            keywordList.insert(keyword[i]);
        }
    }
}

void publishData::buildInstrumentList(std::vector<utils::InstrumtntID> const &nameVec)
{
    for (int i = 0; i < nameVec.size(); i++)
    {
        tickDataPool tempData;
        tempData.id = nameVec[i];
        if (instrumentList.find(tempData) == end(instrumentList))
        {
            tempData.index = instrumentList.size();
            instrumentList.insert(tempData);
        }
    }
}

std::vector<std::string> publishData::getKeywordList(void)
{
    std::vector<std::string> keyword_vector;
    keyword_vector.clear();
    auto iter = keywordList.begin();
    while (iter != keywordList.end())
    {
        keyword_vector.push_back(*iter);
        iter++;
    }

    return keyword_vector;
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
