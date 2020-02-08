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

//自定义头文件
#include "Std_Types.h"
#include "MarketMainFunc.h"
#include "BuildContractDataPool.h"
#include "getconfig.h"

//通用容器
#include <vector>

using namespace std;

//Contracts pool that need to smoke
vector<string> contractPool;


//The contracts that need to be smoked are stored in the array
void BuildContractArray(void)
{
    string str;

    string g_chPoolPath = getConfig("market", "PoolPath");
    
    ifstream ifs(g_chPoolPath.c_str());

    if( ifs.is_open() )   
    { 
        while(getline(ifs,str))
        {        
            contractPool.push_back(str);
        }
    }
    else
    {
        ERROR_LOG("open file error."); 
    }

    ifs.clear(); 
    ifs.close();

}

void InsertDataToContractPool(MARKET_MSG * sM, CThostFtdcDepthMarketDataField * pD)
{    
    for(int i = 0;i < contractPool.size();i++)
    {
        if( strncmp( pD->InstrumentID , contractPool[i].c_str() , contractPool[i].size()-1 ) == 0 )
        {
            //INFO_LOG("len(pD->InstrumentID):%ld, len(contract[%d]):%ld\n",sizeof(pD->InstrumentID),i,sizeof(contractPool[i].c_str()));
            //INFO_LOG("contract1:%s, last price:%2lf.",pD->InstrumentID,pD->LastPrice);
            memcpy(&(sM->datafield[i]),pD,sizeof(CThostFtdcDepthMarketDataField));
            break;
        }
    }
}


