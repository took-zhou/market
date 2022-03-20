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
    auto& jsonCfg = utils::JsonConfig::getInstance();
    string isPrint = jsonCfg.getConfig("common","PrintNetworkDelay").get<std::string>();
    if (strcmp(isPrint.c_str(), "yes") == 0)
    {
        printNetworkDelay = true;
    }
    else
    {
        printNetworkDelay = false;
    }

    md_Instrument_Exhange.clear();
}

bool marketData::isValidTickData(CThostFtdcDepthMarketDataField * pD)
{
    tm tick_tm;
    bool ret = false;
    char UpdateTime[27];

    utils::gbk2utf8(pD->UpdateTime,UpdateTime,sizeof(UpdateTime));
    strptime(UpdateTime, "%H:%M:%S", &tick_tm);

    int tickSecond = tick_tm.tm_hour*60*60 + tick_tm.tm_min*60 + tick_tm.tm_sec;
    //system time
    time_t now_time = time(NULL);
    //local time
    tm* local_time = localtime(&now_time);
    int nowSecond = local_time->tm_hour*60*60 + local_time->tm_min*60 + local_time->tm_sec;

    int delaySecond = nowSecond - tickSecond;
    if (printNetworkDelay == true && delaySecond != 0)
    {
        INFO_LOG("%s delaySecond %d", pD->InstrumentID, delaySecond);
    }

    if (delaySecond <= 180 && delaySecond >= -180 && pD->BidVolume1 > 0 && \
        pD->AskVolume1 > 0 && pD->BidPrice1 > 0.0 && pD->AskPrice1 > 0.0)
    {
        ret = true;
    }

    return ret;
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
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    char buffer[80] = {0};
    struct tm nowTime;
    localtime_r(&curTime.tv_sec, &nowTime);//把得到的值存入临时分配的内存中，线程安全
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &nowTime);

    sprintf(t_arr, "%s.%03d", buffer, milli);
}

bool marketData::getLocalTime(long &stamp)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    stamp = tv.tv_sec;
}

bool marketData::getAssemblingTime(char *t_arr, CThostFtdcDepthMarketDataField * pD)
{
    int y,m,d;
    time_t now_time = time(NULL);
    //local time
    tm* local_time = localtime(&now_time);

    y = 1900+local_time->tm_year;
    m = 1 + local_time->tm_mon;
    d = local_time->tm_mday;

    sprintf(t_arr, "%04d-%02d-%02d %s.%d", y, m, d, pD->UpdateTime, pD->UpdateMillisec);
}
