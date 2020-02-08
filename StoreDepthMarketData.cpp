// CTP头文件
#include <ThostFtdcTraderApi.h>
#include <ThostFtdcMdApi.h>
#include <ThostFtdcUserApiDataType.h>
#include <ThostFtdcUserApiStruct.h>

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

// 字符串编码转化
#include <code_convert.h>

//数据库
#include <mysql/mysql.h>

// 文件夹及文件操作相关
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

//自定义头文件
#include "Std_Types.h"
#include "MarketMainFunc.h"
#include "StoreDepthMarketData.h"

char dataflow[1000];
char sqlquery[2000];
char titleflow[]={"TradingDay,InstrumentID,ExchangeID,ExchangeInstID,\
    LastPrice,PreSettlementPrice,PreClosePrice,PreOpenInterest,OpenPrice,HighestPrice,LowestPrice,\
    Volume,\
    Turnover,OpenInterest,ClosePrice,SettlementPrice,UpperLimitPrice,LowerLimitPrice,PreDelta,CurrDelta,\
    UpdateTime,UpdateMillisec,\
    BidPrice1,BidVolume1,AskPrice1,AskVolume1,\
    BidPrice2,BidVolume2,AskPrice2,AskVolume2,\
    BidPrice3,BidVolume3,AskPrice3,AskVolume3,\
    BidPrice4,BidVolume4,AskPrice4,AskVolume4,\
    BidPrice5,BidVolume5,AskPrice5,AskVolume5,\
    AveragePrice,ActionDay,timestamp"};
extern  vector<string> md_InstrumentID;

static void FormDepthMarketData2Stringflow(CThostFtdcDepthMarketDataField * pD);
static double max2zero(double num);


static double max2zero(double num)
{
    double re;
    if(num >= 100000000 ){re = 0;}
    else{re = num;}
    return re;
}
/*create table if not exists CTP_5S(TradingDay varchar(27),InstrumentID varchar(93),ExchangeID varchar(27),
    ExchangeInstID varchar(93),LastPrice numeric(15,2),PreSettlementPrice numeric(15,2),PreClosePrice numeric(15,2),
    PreOpenInterest numeric(15,2),OpenPrice numeric(15,2),HighestPrice numeric(15,2),LowestPrice numeric(25,2),
    Volume numeric(15),Turnover numeric(15,2),OpenInterest numeric(15,2),ClosePrice numeric(15,2),SettlementPrice numeric(15,2),
    UpperLimitPrice numeric(15,2),LowerLimitPrice numeric(15,2),PreDelta numeric(15,2),CurrDelta numeric(15,2),
    UpdateTime varchar(27),UpdateMillisec numeric(15),BidPrice1 numeric(15,2),BidVolume1 numeric(15),AskPrice1 numeric(15,2),
    AskVolume1 numeric(15),BidPrice2 numeric(15,2),BidVolume2 numeric(15),AskPrice2 numeric(15,2),AskVolume2 numeric(15),
    BidPrice3 numeric(15,2),BidVolume3 numeric(15),AskPrice3 numeric(15,2),AskVolume3 numeric(15),BidPrice4 numeric(15,2),
    BidVolume4 numeric(15),AskPrice4 numeric(15,2),AskVolume4 numeric(15),BidPrice5 numeric(15,2),BidVolume5 numeric(15),
    AskPrice5 numeric(15,2),AskVolume5 numeric(15),AveragePrice numeric(15,2),ActionDay varchar(27))
*/ 
static void FormDepthMarketData2Stringflow(CThostFtdcDepthMarketDataField * pD)
{
    char TradingDay[27];
    gbk2utf8(pD->TradingDay,TradingDay,sizeof(TradingDay));
    ///合约代码 TThostFtdcInstrumentIDType char[31]
    char InstrumentID[93];
    gbk2utf8(pD->InstrumentID,InstrumentID,sizeof(InstrumentID));
    ///交易所代码 TThostFtdcExchangeIDType char[9]
    char ExchangeID[27];
    gbk2utf8(pD->ExchangeID,ExchangeID,sizeof(ExchangeID));
    ///合约在交易所的代码 TThostFtdcExchangeInstIDType char[31]
    char ExchangeInstID[93];
    gbk2utf8(pD->ExchangeInstID,ExchangeInstID,sizeof(ExchangeInstID));
    ///最新价 TThostFtdcPriceType double
    double LastPrice = max2zero(pD->LastPrice);
    ///上次结算价 TThostFtdcPriceType double
    double PreSettlementPrice = max2zero(pD->PreSettlementPrice);
    ///昨收盘 TThostFtdcPriceType double
    double PreClosePrice = max2zero(pD->PreClosePrice);
    ///昨持仓量 TThostFtdcLargeVolumeType double
    double PreOpenInterest = max2zero(pD->PreOpenInterest);
    ///今开盘 TThostFtdcPriceType double
    double OpenPrice = max2zero(pD->OpenPrice);
    ///最高价 TThostFtdcPriceType double
    double HighestPrice = max2zero(pD->HighestPrice);
    ///最低价 TThostFtdcPriceType double
    double LowestPrice = max2zero(pD->LowestPrice);
    ///数量 TThostFtdcVolumeType int
    int Volume = pD->Volume;
    ///成交金额 TThostFtdcMoneyType double
    double Turnover = max2zero(pD->Turnover);
    ///持仓量 TThostFtdcLargeVolumeType double
    double OpenInterest = max2zero(pD->OpenInterest);
    ///今收盘 TThostFtdcPriceType double
    double ClosePrice = max2zero(pD->ClosePrice);
    ///本次结算价 TThostFtdcPriceType double
    double SettlementPrice = max2zero(pD->SettlementPrice);
    ///涨停板价 TThostFtdcPriceType double
    double UpperLimitPrice = max2zero(pD->UpperLimitPrice);
    ///跌停板价 TThostFtdcPriceType double
    double LowerLimitPrice = max2zero(pD->LowerLimitPrice);
    ///昨虚实度 TThostFtdcRatioType double
    double PreDelta = max2zero(pD->PreDelta);
    ///今虚实度 TThostFtdcRatioType double
    double CurrDelta = max2zero(pD->CurrDelta);
    ///最后修改时间 TThostFtdcTimeType char[9]
    char UpdateTime[27];
    gbk2utf8(pD->UpdateTime,UpdateTime,sizeof(UpdateTime));
    ///最后修改毫秒 TThostFtdcMillisecType int
    int UpdateMillisec = pD->UpdateMillisec;
    ///申买价一 TThostFtdcPriceType double
    double BidPrice1 = max2zero(pD->BidPrice1);
    ///申买量一 TThostFtdcVolumeType int
    int BidVolume1 = pD->BidVolume1;
    ///申卖价一 TThostFtdcPriceType double
    double AskPrice1 = max2zero(pD->AskPrice1);
    ///申卖量一 TThostFtdcVolumeType int
    int AskVolume1 = pD->AskVolume1;
    ///申买价二 TThostFtdcPriceType double
    double BidPrice2 = max2zero(pD->BidPrice2);
    ///申买量二 TThostFtdcVolumeType int
    int BidVolume2 = pD->BidVolume2;
    ///申卖价二 TThostFtdcPriceType double
    double AskPrice2 = max2zero(pD->AskPrice2);
    ///申卖量二 TThostFtdcVolumeType int
    int AskVolume2 = pD->AskVolume2;
    ///申买价三 TThostFtdcPriceType double
    double BidPrice3 = max2zero(pD->BidPrice3);
    ///申买量三 TThostFtdcVolumeType int
    int BidVolume3 = pD->BidVolume3;
    ///申卖价三 TThostFtdcPriceType double
    double AskPrice3 = max2zero(pD->AskPrice3);
    ///申卖量三 TThostFtdcVolumeType int
    int AskVolume3 = pD->AskVolume3;
    ///申买价四 TThostFtdcPriceType double
    double BidPrice4 = max2zero(pD->BidPrice4);
    ///申买量四 TThostFtdcVolumeType int
    int BidVolume4 = pD->BidVolume4;
    ///申卖价四 TThostFtdcPriceType double
    double AskPrice4 = max2zero(pD->AskPrice4);
    ///申卖量四 TThostFtdcVolumeType int
    int AskVolume4 = pD->AskVolume4;
    ///申买价五 TThostFtdcPriceType double
    double BidPrice5 = max2zero(pD->BidPrice5);
    ///申买量五 TThostFtdcVolumeType int
    int BidVolume5 = pD->BidVolume5;
    ///申卖价五 TThostFtdcPriceType double
    double AskPrice5 = max2zero(pD->AskPrice5);
    ///申卖量五 TThostFtdcVolumeType int
    int AskVolume5 = pD->AskVolume5;
    ///当日均价 TThostFtdcPriceType double
    double AveragePrice = max2zero(pD->AveragePrice);
    ///业务日期 TThostFtdcDateType char[9]
    char ActionDay[27];
    gbk2utf8(pD->ActionDay,ActionDay,sizeof(ActionDay));
    struct timeval tv;
    gettimeofday(&tv,NULL);
    long timestamp = tv.tv_sec;
    sprintf(dataflow,"'%s','%s','%s','%s', %.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf, %d, %.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf, '%s', %d,\
%.2lf,%d,%.2lf,%d, %.2lf,%d,%.2lf,%d, %.2lf,%d,%.2lf,%d, %.2lf,%d,%.2lf,%d, %.2lf,%d,%.2lf,%d, %.2lf,'%s', %ld",TradingDay,InstrumentID,ExchangeID,ExchangeInstID,\
    LastPrice,PreSettlementPrice,PreClosePrice,PreOpenInterest,OpenPrice,HighestPrice,LowestPrice,\
    Volume,\
    Turnover,OpenInterest,ClosePrice,SettlementPrice,UpperLimitPrice,LowerLimitPrice,PreDelta,CurrDelta,\
    UpdateTime,UpdateMillisec,\
    BidPrice1,BidVolume1,AskPrice1,AskVolume1,\
    BidPrice2,BidVolume2,AskPrice2,AskVolume2,\
    BidPrice3,BidVolume3,AskPrice3,AskVolume3,\
    BidPrice4,BidVolume4,AskPrice4,AskVolume4,\
    BidPrice5,BidVolume5,AskPrice5,AskVolume5,\
    AveragePrice,ActionDay,timestamp);    
}


/*
Requests the trade segment for the current trade contracts
*/
void LoadTradingContracts(vector<string> *vec)
{

    #if 0
    
    MESSAGEHEAD head;
    json reqstatus;

    reqstatus["status"] = 1;
    string msgbody = reqstatus.dump();
    head.datatypeid = MARKETREQINSTRUMENTID;
    head.fromclientname = MARKETNAME;
    head.toclientname = TRADENAME;
    head.length = sizeof(msgbody);
    
    socket_write_msg(head, &msgbody);	
    #endif
    vec->push_back("TA004");
    vec->push_back("TA005");
    vec->push_back("TA006");
    vec->push_back("MA004");
    vec->push_back("MA005");
    vec->push_back("MA006");
    vec->push_back("ZC004");
    vec->push_back("ZC005");
    vec->push_back("ZC006");
    vec->push_back("FG004");
    vec->push_back("FG005");
    vec->push_back("FG006");
    vec->push_back("v2004");
    vec->push_back("v2005");
    vec->push_back("v2006");
    vec->push_back("pp2004");
    vec->push_back("pp2005");
    vec->push_back("pp2006");
    vec->push_back("ag2004");
    vec->push_back("ag2005");
    vec->push_back("ag2006");
    vec->push_back("rb2004");
    vec->push_back("rb2005");
    vec->push_back("rb2006");
}

void LoadDepthMarketDataToMysql(void)
{   
    #if 0
    static int tenCount=0;
    tenCount++;
    if( tenCount%10 == 0 )
    {
        MYSQL conn;
        int res;
        mysql_init(&conn);
        char InstrumentID[93];
        CThostFtdcDepthMarketDataField * temp_pD;
        if( mysql_real_connect(&conn,"localhost","root","sCARbo12","futures",0,NULL,CLIENT_FOUND_ROWS) )
        {
            for(int i=0 ; i<MARKET_BUF_SIZE ; i++)
            {
                temp_pD = &(markH->share_msg->buf[i]);
                if( assert_pD(temp_pD) )  //先判断当前指针结构体是否正确  
                {
                    break;
                }
                gbk2utf8(temp_pD->InstrumentID,InstrumentID,sizeof(InstrumentID));//合约代码
     
                sprintf(sqlquery,"create table if not exists %s(TradingDay varchar(27),InstrumentID varchar(93),ExchangeID varchar(27),\
                ExchangeInstID varchar(93),LastPrice numeric(15,2),PreSettlementPrice numeric(15,2),PreClosePrice numeric(15,2),\
                PreOpenInterest numeric(15,2),OpenPrice numeric(15,2),HighestPrice numeric(15,2),LowestPrice numeric(25,2),\
                Volume numeric(15),Turnover numeric(15,2),OpenInterest numeric(15,2),ClosePrice numeric(15,2),SettlementPrice numeric(15,2),\
                UpperLimitPrice numeric(15,2),LowerLimitPrice numeric(15,2),PreDelta numeric(15,2),CurrDelta numeric(15,2),\
                UpdateTime varchar(27),UpdateMillisec numeric(15),BidPrice1 numeric(15,2),BidVolume1 numeric(15),AskPrice1 numeric(15,2),\
                AskVolume1 numeric(15),BidPrice2 numeric(15,2),BidVolume2 numeric(15),AskPrice2 numeric(15,2),AskVolume2 numeric(15),\
                BidPrice3 numeric(15,2),BidVolume3 numeric(15),AskPrice3 numeric(15,2),AskVolume3 numeric(15),BidPrice4 numeric(15,2),\
                BidVolume4 numeric(15),AskPrice4 numeric(15,2),AskVolume4 numeric(15),BidPrice5 numeric(15,2),BidVolume5 numeric(15),\
                AskPrice5 numeric(15,2),AskVolume5 numeric(15),AveragePrice numeric(15,2),ActionDay varchar(27),TimeStamp numeric(15,2)",InstrumentID);//合成存储路径
        
                res = mysql_query(&conn, sqlquery); 

                FormDepthMarketData2Stringflow(temp_pD);
            
                sprintf(sqlquery,"insert into %s values(%s)",InstrumentID,dataflow);

                res = mysql_query(&conn, sqlquery); 
            
            }
        }
        else
        {
            INFO_LOG("connect error!\n");
            fflush(stdout);

        }
   
        mysql_close(&conn);
    }
    #endif
}

void LoadDepthMarketDataToCsv(CThostFtdcDepthMarketDataField * pD)
{
    char csvpath[200];
    char folderpath[180];
    char InstrumentID[93];
    char ExchangeID[27];
    uint8 existFlag = 1;
    
    if(access("../../../history_data_from_ctp",F_OK) == -1)
    {
        mkdir("../../../history_data_from_ctp",S_IRWXU);
    }
          
    //gbk2utf8(pD->ExchangeID,ExchangeID,sizeof(ExchangeID));//交易所简称   
    gbk2utf8(pD->InstrumentID,InstrumentID,sizeof(InstrumentID));//合约代码
    sprintf(csvpath,"../../../history_data_from_ctp/%s.csv",InstrumentID);//合成存储路径

    //sprintf(csvpath,"../../../history_data_from_ctp/%s/%s.csv",ExchangeID,InstrumentID);//合成存储路径
    //sprintf(folderpath,"../../../history_data_from_ctp/%s",ExchangeID);//合成存储文件夹路径

    #if 0
    if(access(folderpath,F_OK) == -1)
    {
        if(mkdir(folderpath,S_IRWXU) == 0)
        {
            INFO_LOG("create folder %s ok.\n",folderpath);
            fflush(stdout);
        }
        else
        {
            INFO_LOG("create folder %s fail.\n",folderpath);
            fflush(stdout);
        }   
    }
    #endif
    FormDepthMarketData2Stringflow(pD);

    if(access(csvpath,F_OK) == -1)
    {
        existFlag=0;
    }
    ofstream out(csvpath,ios::app);
    if(out.is_open())
    {
        if( existFlag == 0 )
        {
            out<< titleflow << "\n";
        }
        out<< dataflow << "\n";
    }
    else
    {
        INFO_LOG("%s open fail.\n",csvpath);
        fflush(stdout);
    }

    out.close();//关闭文件  
}


