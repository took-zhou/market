#include <stdio.h>  
#include <stdlib.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <string.h>  
#include <unistd.h>
#include <ThostFtdcTraderApi.h>
#include <ThostFtdcMdApi.h>
#include <ThostFtdcUserApiDataType.h>
#include <ThostFtdcUserApiStruct.h>
#include "Std_Types.h"
#include "MarketSocket.h"
#include "MessageHeadId.h"
#include "StoreDepthMarketData.h"
#include "json.h"
#include "getconfig.h"
#include "timer.h"

using json = nlohmann::json;
using namespace std;

bool socketConnectFlag;
int client_sock_fd;
extern TimeoutTimerPool timerPool;

static OSA_STATUS socket_parse_json(MESSAGEHEAD &msgHead, json &msgBody)
{
    int32 byte_num;
    byte_num = recv(client_sock_fd, &msgHead, MESSAGE_HEAD_SZIE, 0);

    if(byte_num < 0)
    {
        ERROR_LOG("read head msg from socket error, recv length:[%d]", byte_num); // @suppress("Invalid arguments")
        return OSA_ERROR;
    }
    if(byte_num == 0)
    {
        ERROR_LOG("router is disconnected!!!!!");
        socketConnectFlag = false;
        return OSA_ERROR;
    }

    char *msgBodyStr = new char[msgHead.length];
    memset(msgBodyStr,0,msgHead.length);

    byte_num = recv(client_sock_fd, msgBodyStr, msgHead.length, 0);
    if(byte_num < 0)
    {
        ERROR_LOG("read body msg from socket error!"); // @suppress("Invalid arguments")
        return OSA_ERROR;
    }
    if(byte_num == 0)
    {
        ERROR_LOG("router is disconnected!!!!!");
        socketConnectFlag = false;
        return OSA_ERROR;
    }

    msgBody = json::parse(msgBodyStr);

    return OSA_OK;
}

//初始化socket
void socket_init(void)
{
    struct sockaddr_in server_addr;

    string g_chSocketPort = getConfig("market", "SocketPort");
    string g_chInetAddr = getConfig("market", "InetAddr");
    int port = atoi(g_chSocketPort.c_str());

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(g_chInetAddr.c_str());
    bzero(&(server_addr.sin_zero), 8);

    client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_sock_fd == -1)
    {
        ERROR_LOG("socket error");
    }

    //建立连接的时候需要传送客户端名称
    while(connect(client_sock_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) == -1)//获取用户输入的字符串并发送给服务器
    {
        INFO_LOG("ipc-socket fail to connect server.");
        sleep(1);
    }

    socketConnectFlag = true;
    INFO_LOG("market init sock ok");

    //写客户端名字
    socket_write_clientName();
}

//读取socket数据，分两次读，先读包头，再读包体，最终输出string
void socket_read_msg(void)
{
    //读包头
    MESSAGEHEAD msgHead;
    json msgBody;
    uint16 dataTypeId;

    while(1)
    {
        if( socketConnectFlag == false )
        {
            sleep(1);
            continue;
        }
        if( socket_parse_json(msgHead, msgBody) == OSA_ERROR )
        {
            ERROR_LOG("socket read error.");
            continue;
        }

        dataTypeId = msgHead.datatypeid;
        //判断接下来的包属于什么数据
        switch( dataTypeId )
        {
            case TRADE_INSTRUMENT_ID:
                //LoadTradingContracts();
                break;
            case CLIENT_HEARTBEAT_ID:
                timerPool.getTimerByName(MARKET_HEADBEAT_TIMER)->stop();
                timerPool.getTimerByName(MARKET_HEADBEAT_TIMER)->restart();
                INFO_LOG("read CLIENT_HEARTBEAT_ID.");
                break;
            default:
                WARNING_LOG("unregistered socket ID: %d!", dataTypeId);
                break;
        }
    }
}

//请求获取当前正在交易的合约信息
void socket_req_instruments(void)
{
    MESSAGEHEAD head;
    json reqstatus;

    reqstatus["status"] = 1;
    string msgbody = reqstatus.dump() + '\0';
    head.datatypeid = MARKET_REQ_INSTRUMENT_ID;
    strcpy(head.fromclientname, MARKETNAME);
    strcpy(head.toclientname, TRADENAME);
    head.length = msgbody.size();

    if( socketConnectFlag == true )
    {
        send(client_sock_fd, &head, MESSAGE_HEAD_SZIE, MSG_NOSIGNAL);
        send(client_sock_fd, msgbody.c_str(), msgbody.size(), MSG_NOSIGNAL);
    }
}

//发送客户端名称
void socket_write_clientName(void)
{
    json headName;
    headName["name"] = MARKETNAME;
    string namestr = headName.dump() + '\0';

    MESSAGEHEAD clientName;
    clientName.datatypeid = 0x0001;//代表客户端的名称
    strcpy(clientName.fromclientname, MARKETNAME);
    strcpy(clientName.toclientname, ROUTENAME);
    clientName.length = namestr.size();

    if( socketConnectFlag == true )
    {
        send(client_sock_fd, &clientName, MESSAGE_HEAD_SZIE, MSG_NOSIGNAL);
        send(client_sock_fd, namestr.c_str(), namestr.size(), MSG_NOSIGNAL);
    }
}

OSA_STATUS socket_write_heartBeat(void)
{
    json heartBeat;
    heartBeat["status"] = "alive";

    string heartBeatstr = heartBeat.dump() + '\0';
    MESSAGEHEAD heartBeatHead;
    heartBeatHead.datatypeid = CLIENT_HEARTBEAT_ID;//代表客户端的名称
    strcpy(heartBeatHead.fromclientname, MARKETNAME);
    strcpy(heartBeatHead.toclientname, MARKETNAME);
    heartBeatHead.length = heartBeatstr.size();

    while(1)
    {
        sleep(15);
        if( socketConnectFlag == true )
        {
            send(client_sock_fd, &heartBeatHead, MESSAGE_HEAD_SZIE, MSG_NOSIGNAL);
            send(client_sock_fd, heartBeatstr.c_str(), heartBeatstr.size(), MSG_NOSIGNAL);
        }
    }
    return OSA_OK;
}

void socket_close(void)
{
    INFO_LOG("market socket is closed.");
    close(client_sock_fd);
}


