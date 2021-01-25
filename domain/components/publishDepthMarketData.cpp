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

void publishData::once(void)
{
    char timeArray[100] = {0};
    market_strategy::message tick;
    auto tick_data = tick.mutable_tick_data();

    getLocalTime(timeArray);
    tick_data->set_time_point(timeArray);

    int ik = pthread_mutex_lock(&(tickData->sm_mutex));
    for (int i = 0; i < instrumentList.size(); i++)
    {
        if (isValidTickData(&tickData->datafield[i]) == false)
        {
            ik = pthread_mutex_unlock(&(tickData->sm_mutex));
            return;
        }

        auto iter = tick_data->add_tick_list();
        iter->set_state(market_strategy::TickData_TickState_active);
        iter->set_instrument_id(tickData->datafield[i].InstrumentID);

        if (keywordList.find("LastPrice") != end(keywordList))
        {
            auto lastPrice = iter->mutable_last_price();
            lastPrice->set_value(std::to_string(max2zero(tickData->datafield[i].LastPrice)));
        }
        if (keywordList.find("BidPrice1") != end(keywordList))
        {
            auto bidPrice1 = iter->mutable_bid_price1();
            bidPrice1->set_value(std::to_string(max2zero(tickData->datafield[i].BidPrice1)));
        }
        if (keywordList.find("BidVolume1") != end(keywordList))
        {
            auto bidVolume1 = iter->mutable_ask_volume1();
            bidVolume1->set_value(tickData->datafield[i].BidVolume1);
        }
        if (keywordList.find("AskPrice1") != end(keywordList))
        {
            auto askPrice1 = iter->mutable_ask_price1();
            askPrice1->set_value(std::to_string(max2zero(tickData->datafield[i].AskPrice1)));
        }
        if (keywordList.find("AskVolume1") != end(keywordList))
        {
            auto askVolume1 = iter->mutable_ask_volume1();
            askVolume1->set_value(tickData->datafield[i].AskVolume1);
        }
        if (keywordList.find("Turnover") != end(keywordList))
        {
            auto turnOver = iter->mutable_turnover();
            turnOver->set_value(tickData->datafield[i].Turnover);
        }
        if (keywordList.find("OpenInterest") != end(keywordList))
        {
            auto openInterest = iter->mutable_open_interest();
            openInterest->set_value(tickData->datafield[i].OpenInterest);
        }
        if (keywordList.find("UpperLimitPrice") != end(keywordList))
        {
            auto upperLimitPrice = iter->mutable_upper_limit_price();
            upperLimitPrice->set_value(std::to_string(max2zero(tickData->datafield[i].UpperLimitPrice)));
        }
        if (keywordList.find("LowerLimitPrice") != end(keywordList))
        {
            auto lowerLimitPrice = iter->mutable_lower_limit_price();
            lowerLimitPrice->set_value(std::to_string(max2zero(tickData->datafield[i].LowerLimitPrice)));
        }
        if (keywordList.find("OpenPrice") != end(keywordList))
        {
            auto openPrice = iter->mutable_open_price();
            openPrice->set_value(std::to_string(max2zero(tickData->datafield[i].OpenPrice)));
        }
        if (keywordList.find("PreSettlementPrice") != end(keywordList))
        {
            auto preSettleMentPrice = iter->mutable_pre_settlement_price();
            preSettleMentPrice->set_value(std::to_string(max2zero(tickData->datafield[i].PreSettlementPrice)));
        }
        if (keywordList.find("PreClosePrice") != end(keywordList))
        {
            auto preClosePrice = iter->mutable_pre_close_price();
            preClosePrice->set_value(std::to_string(max2zero(tickData->datafield[i].PreClosePrice)));
        }
        if (keywordList.find("PreOpenInterest") != end(keywordList))
        {
            auto preOpenInterest = iter->mutable_pre_open_interest();
            preOpenInterest->set_value(tickData->datafield[i].PreOpenInterest);
        }
        if (keywordList.find("Volume") != end(keywordList))
        {
            auto volume = iter->mutable_volume();
            volume->set_value(tickData->datafield[i].Volume);
        }
    }
    ik = pthread_mutex_unlock(&(tickData->sm_mutex));
    std::string tickStr;
    tick.SerializeToString(&tickStr);

    auto& recerSender = RecerSender::getInstance();
    recerSender.ROLE(Sender).ROLE(ProxySender).send("market_strategy.TickData", tickStr.c_str());
}

void publishData::publishToStrategy(void)
{
    if (thread_uniqueness_cnt++ == 0)
    {
        while (1)
        {
            if (indication == market_strategy::TickStartStopIndication_MessageType_start)
            {
                auto& marketSer = MarketService::getInstance();
                if (marketSer.ROLE(Market).ROLE(MarketLoginState).output.status == LOGIN_TIME)
                {
                    once();
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
                break;
            }
            sleep(interval);
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
        memcpy(&(tickData->datafield[iter->index]), pD, sizeof(CThostFtdcDepthMarketDataField));
    }
    ik = pthread_mutex_unlock(&(tickData->sm_mutex));
}

void publishData::setInterval(int _interval)
{
    interval = _interval;
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