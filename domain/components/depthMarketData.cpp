// 线程控制相关
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// 定时器相关
#include <signal.h>
#include <sys/time.h>

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

//自定义头文件
#include "market/domain/components/depthMarketData.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "common/extern/log/log.h"

using namespace std;


marketData::marketData()
{
    md_Instrument_Exhange.clear();
}

bool marketData::isValidTickData(CThostFtdcDepthMarketDataField * pD)
{
    string tickTime = string(pD->UpdateTime);
    vector<string> timeVec = utils::splitString(tickTime,  ":");
    int tickSecond = atoi(timeVec[0].c_str())*60*60 + atoi(timeVec[1].c_str())*60 + atoi(timeVec[2].c_str());
    //system time
    time_t now_time = time(NULL);
    //local time
    tm* local_time = localtime(&now_time);
    int nowSecond = local_time->tm_hour*60*60 + local_time->tm_min*60 + local_time->tm_sec;

    if (abs(tickSecond - nowSecond) <= 60*3)
    {
        return true;
    }
    else
    {
        WARNING_LOG("instrument:%s invalid tick data, ctp time:%dH-%dM-%dS, local time:%dH-%dM-%dS", 
            pD->InstrumentID, atoi(timeVec[0].c_str()), atoi(timeVec[1].c_str()), atoi(timeVec[2].c_str()), 
            local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
        return false;
    }
}

bool marketData::insertInsExchPair(const std::string &ins, const std::string &exch)
{
    auto iter = md_Instrument_Exhange.find(ins);
    if(iter != md_Instrument_Exhange.end())
    {
        if (md_Instrument_Exhange[ins] != exch)
        {
            md_Instrument_Exhange.insert(pair<string, string>(ins, exch));
        }
    }
    else
    {
        md_Instrument_Exhange.insert(pair<string, string>(ins, exch));
    }
    return true;
}

double marketData::max2zero(double num)
{
    double re;
    if (num >= 100000000)
    {
        re = 0;
    }
    else
    {
        re = num;
    }
    return re;
}

std::string marketData::findExchange(std::string ins)
{
    auto iter = md_Instrument_Exhange.find(ins);
    if(iter != md_Instrument_Exhange.end())
    {
        return md_Instrument_Exhange[ins];
    }
    else
    {
        return "";
    }
}

bool marketData::getLocalTime(char *t_arr)
{
    //system time
    time_t now_time = time(NULL);
    //local time
    tm* local_time = localtime(&now_time);
    sprintf(t_arr, "%d-%02d-%02d %02d:%02d:%02d",
        local_time->tm_year + 1900,
        local_time->tm_mon + 1,
        local_time->tm_mday,
        local_time->tm_hour,
        local_time->tm_min,
        local_time->tm_sec);
}

bool marketData::getLocalTime(long &stamp)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    stamp = tv.tv_sec;
}
