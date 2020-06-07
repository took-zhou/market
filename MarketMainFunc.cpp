// 线程控制相关
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <assert.h>
#include <thread>
#include <fcntl.h>

// 定时器相关
#include <signal.h>
#include <sys/time.h>
#include <time.h>

// 字符串编码转化
#include <code_convert.h>
#include "Std_Types.h"
#include <mysql/mysql.h>

//共享内存
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "MarketMainFunc.h"
#include "ctp_timer.h"
#include <vector>

#include "getconfig.h"
#include "StoreDepthMarketData.h"
#include "BuildContractDataPool.h"
#include "MarketSocket.h"
#include "keyboard.h"
#include "timer.h"
#include "MarketLoginControl.h"        // Model's header file
#include "rtwtypes.h"

#define LOGIN_TIME (uint8)1
#define LOGOUT_TIME (uint8)2


// 线程同步标志
sem_t sem;
vector<string> md_InstrumentID;
extern int client_sock_fd;
uint8 login_status = LOGOUT_TIME;


//Create an empty class
CMdHandler *markH;
static MarketLoginControlModelClass rtObj;// Instance of model class

static OSA_STATUS login_process(void);
static void delete_file(void);
std::vector<std::string> split(std::string str,std::string pattern);
void rt_OneStep();
static void one_second_task(void);

/*
------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status
0x7a150008 229379     zhoufan    666        417792     1

*/
//异常退出处理
void signal_exit_handler(int sig)
{
    exit(0);
}

//The constructor
CMdHandler::CMdHandler(CThostFtdcMdApi *pUserApi)
{
    int shm_id;
    key_t key;
    struct shmid_ds buf1;//Used to remove Shared memory
    pthread_mutexattr_t attr;

    //Write once, read multiple times, mutex before read, mutex between read and write
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    //Create a Shared memory area
    string sharepath = getConfig("market", "ShareMemoryAddr");
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

    if( (shm_id = shmget(key,sizeof(MARKET_MSG),IPC_CREAT|0666)) == -1 )
    {
        ERROR_LOG("create shared memory error.");
        exit(1);
    }

    share_msg = (MARKET_MSG*)shmat(shm_id, NULL, 0);
    if(  (long)share_msg == -1)
    {
        ERROR_LOG("attach shared memory error.");
        exit(1);
    }

    //Initializes the mutex
    pthread_mutex_init(&share_msg->sm_mutex, &attr);

    m_pUserMdApi = pUserApi;

    reConnect = 0;

    INFO_LOG("init share message address ok.");

}

void CMdHandler::OnHeartBeatWarning(int nTimeLapse)
{
    INFO_LOG("i touch OnHeartBeatWarning  %d!",nTimeLapse);
}

// 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
void CMdHandler::OnFrontConnected()
{
    INFO_LOG("OnFrontConnected():is excuted...");
    // 在登出后系统会重新调用OnFrontConnected，这里简单判断并忽略第1次之后的所有调用。
    DEBUG_LOG("reConnect:%d.", reConnect);
    if (reConnect++==0) {
        sem_post(&sem);
    }
}

// 当客户端与交易托管系统建立起通信连接，客户端需要进行登录
void CMdHandler::OnFrontDisconnected(int nReason)
{
    ERROR_LOG("OnFrontDisconnected, ErrorCode:%#x", nReason);
}

void CMdHandler::ReqUserLogin()
{
    CThostFtdcReqUserLoginField reqUserLogin;

    strcpy(reqUserLogin.BrokerID, getConfig("market", "BrokerID").c_str());
    strcpy(reqUserLogin.UserID, getConfig("market", "UserID").c_str());
    strcpy(reqUserLogin.Password, getConfig("market", "Password").c_str());
    INFO_LOG("BrokerID: %s", reqUserLogin.BrokerID);
    INFO_LOG("UserID: %s", reqUserLogin.UserID);
    INFO_LOG("Password: %s", reqUserLogin.Password);

    int num = m_pUserMdApi->ReqUserLogin(&reqUserLogin, 0);
    INFO_LOG("\tlogin num = %d", num);
}

// 当客户端发出登录请求之后，该方法会被调用，通知客户端登录是否成功
void CMdHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TThostFtdcErrorMsgType errormsg;
    gbk2utf8(pRspInfo->ErrorMsg,errormsg,sizeof(errormsg));//报错返回信息

    INFO_LOG("OnRspUserLogin:");
    INFO_LOG("\tErrorCode=[%d], ErrorMsg=[%s]", pRspInfo->ErrorID,
        errormsg);
    INFO_LOG("\tRequestID=[%d], Chain=[%d]", nRequestID, bIsLast);
    if (pRspInfo->ErrorID != 0) {
        // 端登失败，客户端需进行错误处理
        INFO_LOG("\tFailed to login, errorcode=%d errormsg=%s requestid=%d chain = %d",
            pRspInfo->ErrorID, errormsg, nRequestID, bIsLast);
        exit(-1);
    }
    sem_post(&sem);
}

void CMdHandler::ReqUserLogout(void)
{
    CThostFtdcUserLogoutField reqUserLogout;

    strcpy(reqUserLogout.BrokerID, getConfig("market", "BrokerID").c_str());
    strcpy(reqUserLogout.UserID, getConfig("market", "UserID").c_str());

    int num = m_pUserMdApi->ReqUserLogout(&reqUserLogout, 0);
    INFO_LOG("\tlogin num = %d", num);
}

///登出请求响应
void CMdHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TThostFtdcErrorMsgType errormsg;
    gbk2utf8(pRspInfo->ErrorMsg,errormsg,sizeof(errormsg));//报错返回信息

    INFO_LOG("OnRspUserLogout:");
    INFO_LOG("\tErrorCode=[%d], ErrorMsg=[%s]", pRspInfo->ErrorID,
        errormsg);
    INFO_LOG("\tRequestID=[%d], Chain=[%d]", nRequestID, bIsLast);
    if (pRspInfo->ErrorID != 0) {
        // 端登失败，客户端需进行错误处理
        INFO_LOG("\tFailed to logout, errorcode=%d errormsg=%s requestid=%d chain = %d",
            pRspInfo->ErrorID, errormsg, nRequestID, bIsLast);
        exit(-1);
    }
    sem_post(&sem);

}

void CMdHandler::SubscribeMarketData()//收行情
{
    int md_num = 0;
    char **ppInstrumentID = new char*[5000];
    for (int count1 = 0; count1 <= md_InstrumentID.size() / 500; count1++)
    {
        if (count1 < md_InstrumentID.size() / 500)
        {
            int a = 0;
            for (a; a < 500; a++)
            {
                ppInstrumentID[a] = const_cast<char *>(md_InstrumentID.at(md_num).c_str());
                md_num++;
            }
            int result = m_pUserMdApi->SubscribeMarketData(ppInstrumentID, a);
            if( result == 0 )
            {
                INFO_LOG("Subscription request 1......Send a success");
            }
            else
            {
                INFO_LOG("Subscription request 1......Failed to send, error serial number=[%d]", result);
            }
        }
        else if (count1 == md_InstrumentID.size() / 500)
        {
            int count2 = 0;
            for (count2; count2 < md_InstrumentID.size() % 500; count2++)
            {
                ppInstrumentID[count2] = const_cast<char *>(md_InstrumentID.at(md_num).c_str());
                md_num++;
            }
            int result = m_pUserMdApi->SubscribeMarketData(ppInstrumentID, count2);

            if( result == 0 )
            {
                INFO_LOG("Subscription request 2......Send a success");
            }
            else
            {
                INFO_LOG("Subscription request 2......Failed to send, error serial number=[%d]", result);
            }

        }
    }
}

///订阅行情应答
void CMdHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    #if 0
        INFO_LOG("<OnRspSubMarketData>");
        if (pSpecificInstrument)
        {
            INFO_LOG("\tInstrumentID = [%s]", pSpecificInstrument->InstrumentID);
        }
        if (pRspInfo)
        {
            INFO_LOG("\tErrorMsg = [%s]", pRspInfo->ErrorMsg);
            INFO_LOG("\tErrorID = [%d]", pRspInfo->ErrorID);
        }
        INFO_LOG("\tnRequestID = [%d]", nRequestID);
        INFO_LOG("\tbIsLast = [%d]", bIsLast);
        INFO_LOG("</OnRspSubMarketData>");
    #endif
    sem_post(&sem);
}

///深度行情通知
void CMdHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    int ik;
    //INFO_LOG("<OnRtnDepthMarketData>");
    if (pDepthMarketData)
    {
        ik=pthread_mutex_lock(&share_msg->sm_mutex);
        /*Market data is stored in the Shared memory area*/
        InsertDataToContractPool(share_msg, pDepthMarketData);
        /*Store tick data into CSV text*/
        LoadDepthMarketDataToCsv(pDepthMarketData);
        ik = pthread_mutex_unlock(&share_msg->sm_mutex);
    }
    //INFO_LOG("</OnRtnDepthMarketData>");
}

///订阅询价请求
void CMdHandler::SubscribeForQuoteRsp()
{
    INFO_LOG("Quotation subscription inquiry request.");
    char **ppInstrumentID = new char*[50];
    string g_chInstrumentID ;
    //= getConfig("config", "InstrumentID");
    ppInstrumentID[0] = const_cast<char *>(g_chInstrumentID.c_str());
    int result = m_pUserMdApi->SubscribeForQuoteRsp(ppInstrumentID, 1);
}

///订阅询价应答
void CMdHandler::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
    int nRequestID, bool bIsLast)
{
    INFO_LOG("<OnRspSubForQuoteRsp>");
    if (pSpecificInstrument)
    {
        INFO_LOG("\tInstrumentID = [%s]", pSpecificInstrument->InstrumentID);
    }
    if (pRspInfo)
    {
        INFO_LOG("\tErrorMsg = [%s]", pRspInfo->ErrorMsg);
        INFO_LOG("\tErrorID = [%d]", pRspInfo->ErrorID);
    }
    INFO_LOG("\tnRequestID = [%d]", nRequestID);
    INFO_LOG("\tbIsLast = [%d]", bIsLast);
    INFO_LOG("</OnRspSubForQuoteRsp>");
    sem_post(&sem);
}

///询价通知
void CMdHandler::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    INFO_LOG("<OnRtnForQuoteRsp>");
    if (pForQuoteRsp)
    {
        INFO_LOG("\tTradingDay = [%s]", pForQuoteRsp->TradingDay);
        INFO_LOG("\tInstrumentID = [%s]", pForQuoteRsp->InstrumentID);
        INFO_LOG("\tForQuoteSysID = [%s]", pForQuoteRsp->ForQuoteSysID);
        INFO_LOG("\tForQuoteTime = [%s]", pForQuoteRsp->ForQuoteTime);
        INFO_LOG("\tActionDay = [%s]", pForQuoteRsp->ActionDay);
        INFO_LOG("\tExchangeID = [%s]", pForQuoteRsp->ExchangeID);
    }
    INFO_LOG("</OnRtnForQuoteRsp>");
    sem_post(&sem);
}

//等待登出时间
void CMdHandler::WaitLogoutTime(void)
{
    INFO_LOG("wait for logout time.");

    while(1)
    {
        if( login_status == LOGOUT_TIME )
        {
            reConnect = 0;
            DEBUG_LOG("reConnect:%d.", reConnect);
            return;
        }
        else
        {
            sleep(50);
        }
    }
}

static OSA_STATUS login_process(void)
{
    // 实时时间
    time_t now;
    struct tm *timenow;

    // 初始化线程同步变量
    sem_init(&sem,0,0);

    string marketCons = getConfig("market", "ConRelativePath");
    string command = "mkdir -p " + marketCons;
    if(access(marketCons.c_str(),F_OK) == -1)
    {
        system(command.c_str());
        if(access(marketCons.c_str(),F_OK) == 0)
        {
            INFO_LOG("mkdir -p %s ok.", marketCons.c_str());
        }
    }

    CThostFtdcMdApi  *pUserMdApi =
    CThostFtdcMdApi::CreateFtdcMdApi(marketCons.c_str());

    markH = new CMdHandler(pUserMdApi);
    pUserMdApi->RegisterSpi(markH);

    // 设置服务器地址
    string g_chFrontaddr = getConfig("market", "FrontMdAddr");
    pUserMdApi->RegisterFront(const_cast<char *>(g_chFrontaddr.c_str()));

    // 链接交易系统
    pUserMdApi->Init();
    sem_wait(&sem);

    //while( sem_trywait(&sem) ==  EAGAIN )
    //{
    //    time(&now);
    //    timenow = localtime(&now);//获取当前时间
    //    if( timenow->tm_hour >= ENDHOURS )
    //    {
    //        pUserMdApi->Release();
    //        WARNING_LOG("during login time, but cannot login");
    //        return OSA_ERROR;
    //    }
    //    usleep(1000000);
    //}

    markH->ReqUserLogin();
    sem_wait(&sem);

    //创建
    BuildContractArray();

    LoadTradingContracts(&md_InstrumentID);

    markH->SubscribeMarketData();//订阅行情
    sem_wait(&sem);

    markH->WaitLogoutTime();//等待登出时间

    pUserMdApi->Release();
}

int main(int argc,char **argv) {

    rtObj.initialize();

    //初始化log参数
    string logpath = getConfig("market", "LogRelativePath");
    string command = "mkdir -p " + logpath;
    if(access(logpath.c_str(),F_OK) == -1)
    {
        system(command.c_str());
        if(access(logpath.c_str(),F_OK) == 0)
        {
            INFO_LOG("mkdir -p %s ok.", logpath.c_str());
        }
    }
    LOG_INIT(logpath.c_str(), "marketlog", 6);

    // 初始化socket
    socket_init();

    // 创建并启动读socket线程
    thread t1(socket_read_msg);
    t1.detach();

    // 开启心跳线程
    thread t2(socket_write_heartBeat);
    t2.detach();

    // 开启状态转换
    thread t3(one_second_task);

    // 添加计时器
    auto& timerPool= TimeoutTimerPool::getInstance();
    timerPool.addTimer(ROUTE_HEADBEAT_TIMER, socket_reconnect, HEADBEAT_TIME_OUT_LENGTH);

#if 0
{
    BuildContractArray();

    string marketCons = getConfig("market", "ConRelativePath");
    string command = "mkdir -p " + marketCons;
    if(access(marketCons.c_str(),F_OK) == -1)
    {
        system(command.c_str());
        if(access(marketCons.c_str(),F_OK) == 0)
        {
            INFO_LOG("mkdir -p %s ok.", marketCons.c_str());
        }
    }

    CThostFtdcMdApi  *pUserMdApi =
    CThostFtdcMdApi::CreateFtdcMdApi(marketCons.c_str());

    markH = new CMdHandler(pUserMdApi);

    CThostFtdcDepthMarketDataField pDepthMarketData;
    vector<float32> vec1, vec2;

    string con1 = "l1705";
    string con2 = "v1705";

    LoadDepthMarketDataFromMysql(&vec1, con1.c_str());
    LoadDepthMarketDataFromMysql(&vec2, con2.c_str());

    if( vec1.size() == vec2.size() )
    {
        strcpy(pDepthMarketData.ExchangeID, "DCE");
        for(int i=0; i<vec1.size(); i++)
        {
            int ik;

            ik = pthread_mutex_lock(&markH->share_msg->sm_mutex);

            strcpy(pDepthMarketData.InstrumentID, con1.c_str() );
            pDepthMarketData.LastPrice =  vec1[i];
            InsertDataToContractPool(markH->share_msg, &pDepthMarketData);
            INFO_LOG("%.2f", vec1[i]);

            strcpy(pDepthMarketData.InstrumentID, con2.c_str() );
            pDepthMarketData.LastPrice =  vec2[i];
            InsertDataToContractPool(markH->share_msg, &pDepthMarketData);
            INFO_LOG("%.2f", vec2[i]);

            ik = pthread_mutex_unlock(&markH->share_msg->sm_mutex);

            usleep(500000);
        }
    }
    else
    {
        ERROR_LOG("get data length is not same");
    }
}
#endif

    while(1)
    {
        if(login_status == LOGIN_TIME)
        {
            login_process();
        }
        else
        {
            sleep(10);
        }
    }

    return(0);
}

//字符串分割函数
std::vector<std::string> split(std::string str,std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str+=pattern;//扩展字符串以方便操作
    int size=str.size();

    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i = pos+pattern.size()-1;
        }
    }
    return result;
}

static void one_second_task(void)
{
    while(1)
    {
        rt_OneStep();
        sleep(1);
    }
}

void rt_OneStep(void)
{
  // 实时时间
  time_t now = {0};
  struct tm *timenow = NULL;

  static boolean_T OverrunFlag = false;

  // Disable interrupts here

  // Check for overrun
  if (OverrunFlag) {
    rtmSetErrorStatus(rtObj.getRTM(), "Overrun");
    return;
  }

  OverrunFlag = true;

  // Save FPU context here (if necessary)
  // Re-enable timer or interrupt here
  // Set model inputs here
  string timeStr = getConfig("market", "DayLoginTime");
  vector<string> timeVec=split(timeStr,  ":");
  rtObj.rtU.day_login_mins = atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());
  //INFO_LOG("rtObj.rtU.day_login_mins:%d", rtObj.rtU.day_login_mins);

  timeStr = getConfig("market", "DayLogoutTime");
  timeVec= split(timeStr,  ":");
  rtObj.rtU.day_logout_mins= atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());
  //INFO_LOG("rtObj.rtU.day_logout_mins:%d", rtObj.rtU.day_logout_mins);

  timeStr = getConfig("market", "NightLoginTime");
  timeVec=split(timeStr,  ":");
  rtObj.rtU.night_login_mins= atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());
  //INFO_LOG("rtObj.rtU.night_login_mins:%d", rtObj.rtU.night_login_mins);

  timeStr = getConfig("market", "NightLogoutTime");
  timeVec=split(timeStr,  ":");
  rtObj.rtU.night_logout_mins= atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());
  //INFO_LOG("rtObj.rtU.night_logout_mins:%d", rtObj.rtU.night_logout_mins);

  timeStr = getConfig("market", "LoginTime");
  strcpy(rtObj.rtU.loginTime, timeStr.c_str());

  time(&now);
  timenow = localtime(&now);//获取当前时间
  rtObj.rtU.now_mins= timenow->tm_hour*60 + timenow->tm_min;
  //INFO_LOG("rtObj.rtU.now_mins:%d", rtObj.rtU.now_mins);

  // Step the model
  rtObj.step();

  // Get model outputs here
  login_status = rtObj.rtY.status;

  // Indicate task complete
  OverrunFlag = false;

  // Disable interrupts here
  // Restore FPU context here (if necessary)
  // Enable interrupts here
}


