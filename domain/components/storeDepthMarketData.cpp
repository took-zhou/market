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
#include "market/domain/components/storeDepthMarketData.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "common/extern/log/log.h"

using namespace std;

loadData::loadData()
{
    auto& jsonCfg = utils::JsonConfig::getInstance();
    history_tick_folder = jsonCfg.getConfig("market","HistoryTickPath").get<std::string>();
}

void loadData::FormDepthMarketData2Stringflow(CThostFtdcDepthMarketDataField * pD)
{
    ///合约代码 TThostFtdcInstrumentIDType char[31]
    char InstrumentID[93];
    utils::gbk2utf8(pD->InstrumentID,InstrumentID,sizeof(InstrumentID));
    ///交易日期
    char TradingDay[27];
    utils::gbk2utf8(pD->TradingDay,TradingDay,sizeof(TradingDay));
    ///最后修改时间 TThostFtdcTimeType char[9]
    char UpdateTime[27];
    utils::gbk2utf8(pD->UpdateTime,UpdateTime,sizeof(UpdateTime));
    ///最后修改毫秒 TThostFtdcMillisecType int
    int UpdateMillisec = pD->UpdateMillisec;
    ///最新价 TThostFtdcPriceType double
    double LastPrice = max2zero(pD->LastPrice);
    ///申买价一 TThostFtdcPriceType double
    double BidPrice1 = max2zero(pD->BidPrice1);
    ///申买量一 TThostFtdcVolumeType int
    int BidVolume1 = pD->BidVolume1;
    ///申卖价一 TThostFtdcPriceType double
    double AskPrice1 = max2zero(pD->AskPrice1);
    ///申卖量一 TThostFtdcVolumeType int
    int AskVolume1 = pD->AskVolume1;
    ///申买价一 TThostFtdcPriceType double
    double BidPrice2 = max2zero(pD->BidPrice2);
    ///申买量一 TThostFtdcVolumeType int
    int BidVolume2 = pD->BidVolume2;
    ///申卖价一 TThostFtdcPriceType double
    double AskPrice2 = max2zero(pD->AskPrice2);
    ///申卖量一 TThostFtdcVolumeType int
    int AskVolume2 = pD->AskVolume2;
    ///申买价一 TThostFtdcPriceType double
    double BidPrice3 = max2zero(pD->BidPrice3);
    ///申买量一 TThostFtdcVolumeType int
    int BidVolume3 = pD->BidVolume3;
    ///申卖价一 TThostFtdcPriceType double
    double AskPrice3 = max2zero(pD->AskPrice3);
    ///申卖量一 TThostFtdcVolumeType int
    int AskVolume3 = pD->AskVolume3;
    ///申买价一 TThostFtdcPriceType double
    double BidPrice4 = max2zero(pD->BidPrice4);
    ///申买量一 TThostFtdcVolumeType int
    int BidVolume4 = pD->BidVolume4;
    ///申卖价一 TThostFtdcPriceType double
    double AskPrice4 = max2zero(pD->AskPrice4);
    ///申卖量一 TThostFtdcVolumeType int
    int AskVolume4 = pD->AskVolume4;
    ///申买价一 TThostFtdcPriceType double
    double BidPrice5 = max2zero(pD->BidPrice5);
    ///申买量一 TThostFtdcVolumeType int
    int BidVolume5 = pD->BidVolume5;
    ///申卖价一 TThostFtdcPriceType double
    double AskPrice5 = max2zero(pD->AskPrice5);
    ///申卖量一 TThostFtdcVolumeType int
    int AskVolume5 = pD->AskVolume5;
    ///数量 TThostFtdcVolumeType int
    int Volume = pD->Volume;
    ///成交金额 TThostFtdcMoneyType double
    double Turnover = max2zero(pD->Turnover);
    ///持仓量 TThostFtdcLargeVolumeType double
    double OpenInterest = max2zero(pD->OpenInterest);
    ///涨停板价 TThostFtdcPriceType double
    double UpperLimitPrice = max2zero(pD->UpperLimitPrice);
    ///跌停板价 TThostFtdcPriceType double
    double LowerLimitPrice = max2zero(pD->LowerLimitPrice);
    ///今开盘 TThostFtdcPriceType double
    double OpenPrice = max2zero(pD->OpenPrice);
    ///上次结算价 TThostFtdcPriceType double
    double PreSettlementPrice = max2zero(pD->PreSettlementPrice);
    ///昨收盘 TThostFtdcPriceType double
    double PreClosePrice = max2zero(pD->PreClosePrice);
    ///昨持仓量 TThostFtdcLargeVolumeType double
    double PreOpenInterest = max2zero(pD->PreOpenInterest);
    ///结算价 SettlementPrice double
    double SettlementPrice = max2zero(pD->SettlementPrice);

    sprintf(dataflow,"%s,%s,%s.%d,%.6lf,%.6lf,%d,%.6lf,%d,%.6lf,%d,%.6lf,%d,%.6lf,%d,%.6lf,%d,%.6lf,%d,%.6lf,%d,%.6lf,%d,%.6lf,%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.5lf,%.6lf",\
    InstrumentID,TradingDay,UpdateTime,UpdateMillisec,LastPrice,BidPrice1,BidVolume1,AskPrice1,AskVolume1,BidPrice2,BidVolume2,AskPrice2,\
    AskVolume2,BidPrice3,BidVolume3,AskPrice3,AskVolume3,BidPrice4,BidVolume4,AskPrice4,AskVolume4,BidPrice5,BidVolume5,AskPrice5,AskVolume5,\
    Volume,Turnover,OpenInterest,UpperLimitPrice,LowerLimitPrice,OpenPrice,PreSettlementPrice,PreClosePrice,PreOpenInterest,SettlementPrice);
}

void loadData::LoadDepthMarketDataToCsv(CThostFtdcDepthMarketDataField * pD)
{
    char csvpath[200];
    char folderpath[180];
    char InstrumentID[93];
    char ExchangeID[27];
    U8 existFlag = 1;

    utils::gbk2utf8(pD->InstrumentID,InstrumentID,sizeof(InstrumentID));//合约代码
    if (InstrumentID[2] == 'e' && InstrumentID[3] == 'f' && InstrumentID[4] == 'p')
    {
        return;
    }

    if (isValidTickData(pD) == false)
    {
        return;
    }

    if(access(history_tick_folder.c_str(), F_OK) == -1)
    {
        mkdir(history_tick_folder.c_str(), S_IRWXU);
    }

    sprintf(csvpath,"%s/%s.csv",history_tick_folder.c_str(), InstrumentID);//合成存储路径

    FormDepthMarketData2Stringflow(pD);

    if(access(csvpath,F_OK) == -1)
    {
        existFlag=0;
    }

    std::ofstream out(csvpath,std::ios::app);
    if(out.is_open())
    {
        if( existFlag == 0 )
        {
            out<< titleflow << "\r\r\n";
        }
        out<< dataflow << "\r\r\n";
    }
    else
    {
        INFO_LOG("%s open fail.\n",csvpath);
        fflush(stdout);
    }

    out.close();//关闭文件  
}

bool loadData::ClassifyContractFiles(void)
{
    struct dirent * filename;    // return value for readdir()
    DIR * dir;                   // return value for opendir()
    dir = opendir(history_tick_folder.c_str());
    if (NULL == dir)
    {
        ERROR_LOG("Can not open dir ");
        return false;
    }

    while ((filename = readdir(dir)) != NULL)
    {
        if (strcmp(filename->d_name , ".") == 0 || strcmp(filename->d_name , "..") == 0)
            continue;

        //在map中查找合约
        string contractFile = filename->d_name;
        int pos = contractFile.find_first_of('.');
        string fileName(contractFile.substr(0,pos));
        string fileFormat(contractFile.substr(pos+1));

        if ((pos == -1) || (strcmp(fileFormat.c_str(), "csv") != 0))
        {
            continue;
        }

        string ins = findExchange(fileName);
        if (ins != "")
        {
            MoveContractToFolder(contractFile, findExchange(fileName));
            usleep(1000);
        }
        else
        {
            WARNING_LOG("not found fileName:%s", fileName.c_str());
        }
    }

    closedir(dir);
    return true;
} 

bool loadData::MoveContractToFolder(string contractName, string exchangeName)
{
    char csvpath[200];
    char folderpath[180];
    char command[400];

    sprintf(csvpath,"%s/%s", history_tick_folder.c_str(), contractName.c_str());//合成存储路径
    sprintf(folderpath,"%s/%s/", history_tick_folder.c_str(), exchangeName.c_str());//合成存储文件夹路径

    if(access(folderpath,F_OK) == -1)
    {
        if(mkdir(folderpath,S_IRWXU) == 0)
        {
            INFO_LOG("create folder %s ok.\n",folderpath);
            fflush(stdout);
        }
        else
        {
            ERROR_LOG("create folder %s fail.\n",folderpath);
            fflush(stdout);
            return false;
        }
    }

    sprintf(command, "mv %s %s", csvpath, folderpath);
    system(command);
    return true;
}
