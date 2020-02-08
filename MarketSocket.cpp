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


using json = nlohmann::json;

#define MESSAGE_HEAD_SZIE (sizeof(MESSAGEHEAD))  

typedef struct
{
    unsigned short datatypeid;
    char fromclientname[20];
    char toclientname[20];
    unsigned short length;
}MESSAGEHEAD;

int client_sock_fd;

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
    INFO_LOG("market init sock ok");
}	

//读取socket数据，分两次读，先读包头，再读包体，最终输出string
void socket_read_msg(void)
{
    //读包头
    MESSAGEHEAD head;
    uint16 datatypeid;
    long byte_num;
    while(1)
    {
        byte_num = recv(client_sock_fd, &head, MESSAGE_HEAD_SZIE, 0);
        datatypeid = head.datatypeid;

        //判断接下来的包属于什么数据
        switch( datatypeid )
        {
            case TRADE_INSTRUMENT_ID:
                //LoadTradingContracts();
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
    send(client_sock_fd, &head, MESSAGE_HEAD_SZIE, 0);
    send(client_sock_fd, msgbody.c_str(), msgbody.size(), 0);
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
    
    send(client_sock_fd, &clientName, MESSAGE_HEAD_SZIE, 0);
    send(client_sock_fd, namestr.c_str(), namestr.size(), 0);
}


