#ifndef MARKET_SOCKET_H
#define MARKET_SOCKET_H




#define MESSAGE_HEAD_SZIE (sizeof(MESSAGEHEAD))
constexpr char MARKET_HEADBEAT_TIMER[]   = "route_headbeat_check";
constexpr uint32  MARKET_HEADBEAT_TIMEOUT_LENGTH = 45 * 1000;


typedef struct
{
    unsigned short datatypeid;
    char fromclientname[20];
    char toclientname[20];
    unsigned short length;
}MESSAGEHEAD;


OSA_STATUS socket_init(void);
void socket_read_msg(void);
void socket_req_instruments(void);
void socket_write_clientName(void);
OSA_STATUS socket_write_heartBeat(void);
void socket_close(void);
OSA_STATUS socket_reconnect(void);

#endif

