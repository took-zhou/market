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
                iter->set_last_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].LastPrice)));
            }
            if (pc->second.keywordList.find("BidPrice1") != end(pc->second.keywordList))
            {
                iter->set_bid_price1(std::to_string(max2zero(tickData->datafield[instrument_iter->index].BidPrice1)));
            }
            if (pc->second.keywordList.find("BidVolume1") != end(pc->second.keywordList))
            {
                iter->set_bid_volume1(tickData->datafield[instrument_iter->index].BidVolume1);
            }
            if (pc->second.keywordList.find("AskPrice1") != end(pc->second.keywordList))
            {
                iter->set_ask_price1(std::to_string(max2zero(tickData->datafield[instrument_iter->index].AskPrice1)));
            }
            if (pc->second.keywordList.find("AskVolume1") != end(pc->second.keywordList))
            {
                iter->set_ask_volume1(tickData->datafield[instrument_iter->index].AskVolume1);
            }
            if (pc->second.keywordList.find("Turnover") != end(pc->second.keywordList))
            {
                iter->set_turnover(tickData->datafield[instrument_iter->index].Turnover);
            }
            if (pc->second.keywordList.find("OpenInterest") != end(pc->second.keywordList))
            {
                iter->set_open_interest(tickData->datafield[instrument_iter->index].OpenInterest);
            }
            if (pc->second.keywordList.find("UpperLimitPrice") != end(pc->second.keywordList))
            {
                iter->set_upper_limit_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].UpperLimitPrice)));
            }
            if (pc->second.keywordList.find("LowerLimitPrice") != end(pc->second.keywordList))
            {
                iter->set_lower_limit_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].LowerLimitPrice)));
            }
            if (pc->second.keywordList.find("OpenPrice") != end(pc->second.keywordList))
            {
                iter->set_open_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].OpenPrice)));
            }
            if (pc->second.keywordList.find("PreSettlementPrice") != end(pc->second.keywordList))
            {
                iter->set_pre_settlement_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreSettlementPrice)));
            }
            if (pc->second.keywordList.find("PreClosePrice") != end(pc->second.keywordList))
            {
                iter->set_pre_close_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreClosePrice)));
            }
            if (pc->second.keywordList.find("PreOpenInterest") != end(pc->second.keywordList))
            {
                iter->set_pre_open_interest(tickData->datafield[instrument_iter->index].PreOpenInterest);
            }
            if (pc->second.keywordList.find("Volume") != end(pc->second.keywordList))
            {
                iter->set_volume(tickData->datafield[instrument_iter->index].Volume);
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
        iter->set_last_price(std::to_string(max2zero(pD->LastPrice)));
    }
    if (pc->second.keywordList.find("BidPrice1") != end(pc->second.keywordList))
    {
        iter->set_bid_price1(std::to_string(max2zero(pD->BidPrice1)));
    }
    if (pc->second.keywordList.find("BidVolume1") != end(pc->second.keywordList))
    {
        iter->set_bid_volume1(pD->BidVolume1);
    }
    if (pc->second.keywordList.find("AskPrice1") != end(pc->second.keywordList))
    {
        iter->set_ask_price1(std::to_string(max2zero(pD->AskPrice1)));
    }
    if (pc->second.keywordList.find("AskVolume1") != end(pc->second.keywordList))
    {
        iter->set_ask_volume1(pD->AskVolume1);
    }
    if (pc->second.keywordList.find("Turnover") != end(pc->second.keywordList))
    {
        iter->set_turnover(pD->Turnover);
    }
    if (pc->second.keywordList.find("OpenInterest") != end(pc->second.keywordList))
    {
        iter->set_open_interest(pD->OpenInterest);
    }
    if (pc->second.keywordList.find("UpperLimitPrice") != end(pc->second.keywordList))
    {
        iter->set_upper_limit_price(std::to_string(max2zero(pD->UpperLimitPrice)));
    }
    if (pc->second.keywordList.find("LowerLimitPrice") != end(pc->second.keywordList))
    {
        iter->set_lower_limit_price(std::to_string(max2zero(pD->LowerLimitPrice)));
    }
    if (pc->second.keywordList.find("OpenPrice") != end(pc->second.keywordList))
    {
        iter->set_open_price(std::to_string(max2zero(pD->OpenPrice)));
    }
    if (pc->second.keywordList.find("PreSettlementPrice") != end(pc->second.keywordList))
    {
        iter->set_pre_settlement_price(std::to_string(max2zero(pD->PreSettlementPrice)));
    }
    if (pc->second.keywordList.find("PreClosePrice") != end(pc->second.keywordList))
    {
        iter->set_pre_close_price(std::to_string(max2zero(pD->PreClosePrice)));
    }
    if (pc->second.keywordList.find("PreOpenInterest") != end(pc->second.keywordList))
    {
        iter->set_pre_open_interest(pD->PreOpenInterest);
    }
    if (pc->second.keywordList.find("Volume") != end(pc->second.keywordList))
    {
        iter->set_volume(pD->Volume);
    }

    std::string tickStr;
    tick.SerializeToString(&tickStr);

    auto& recerSender = RecerSender::getInstance();
    string topic = "market_strategy.TickData." + pc->first;
    recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
}

void publishData::publishToStrategy(const string keyname)
{
    auto& marketSer = MarketService::getInstance();
    std::map<string, publishControl>::iterator iter = marketSer.ROLE(controlPara).publishCtrlMap.find(keyname);
    if (iter != marketSer.ROLE(controlPara).publishCtrlMap.end() && iter->second.thread_uniqueness_cnt++ == 0)
    {
        INFO_LOG("publishDataFuc prepare ok");
        while (1)
        {
            if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_start)
            {
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
                INFO_LOG("%s is going to exit publishToStrategy thread.", iter->first.c_str());
                marketSer.ROLE(controlPara).publishCtrlMap.erase(iter);
                break;
            }
            usleep(iter->second.interval);
        }
    }
}

void publishData::directForwardDataToStrategy(CThostFtdcDepthMarketDataField * pD)
{
    auto& marketSer = MarketService::getInstance();
    tickDataPool tempData;
    tempData.id.ins = pD->InstrumentID;
    std::map<std::string, publishControl>::iterator saveit;
    std::map<std::string, publishControl>::iterator mapit = marketSer.ROLE(controlPara).publishCtrlMap.begin();
    while (mapit != marketSer.ROLE(controlPara).publishCtrlMap.end())
    {
        auto pos = mapit->second.instrumentList.find(tempData);
        if (pos != end(mapit->second.instrumentList) && mapit->second.directforward == true)
        {
            if (mapit->second.indication == market_strategy::TickStartStopIndication_MessageType_start)
            {
                auto& marketSer = MarketService::getInstance();
                if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE)
                {
                    once_from_dataflow(mapit, pD);
                }
            }
            else if (mapit->second.indication == market_strategy::TickStartStopIndication_MessageType_stop)
            {
                INFO_LOG("publishToStrategy is stopping.");
            }
            else if (mapit->second.indication == market_strategy::TickStartStopIndication_MessageType_finish)
            {
                saveit = mapit;
                mapit++;
                INFO_LOG("%s is going to exit publishToStrategy thread.", saveit->first.c_str());
                marketSer.ROLE(controlPara).publishCtrlMap.erase(saveit);
                continue;
            }
        }
        mapit++;
    }
}

void publishData::insertDataToTickDataPool(CThostFtdcDepthMarketDataField * pD)
{
    int ik = pthread_mutex_lock(&(tickData->sm_mutex));
    tickDataPool tempData;
    tempData.id.ins = string(pD->InstrumentID);

    auto& marketSer = MarketService::getInstance();
    auto iter = marketSer.ROLE(controlPara).instrumentList.find(tempData);
    if (iter != marketSer.ROLE(controlPara).instrumentList.end())
    {
        if (isValidTickData(pD) == true)
        {
            memcpy(&(tickData->datafield[iter->index]), pD, sizeof(CThostFtdcDepthMarketDataField));
        }
    }
    ik = pthread_mutex_unlock(&(tickData->sm_mutex));
}
