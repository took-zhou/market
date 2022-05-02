/*
 * traderEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_TRADEREVENT_TRADEREVENT_H_
#define WORKSPACE_MARKET_INTERFACE_TRADEREVENT_TRADEREVENT_H_
#include <map>
#include <functional>
#include <string>

struct MsgStruct;

struct TraderEvent
{
public:
    bool init();
    void handle(MsgStruct& msg);
    void regMsgFun();
    void QryInstrumentRspHandle(MsgStruct& msg);

    std::map<std::string, std::function<void(MsgStruct& msg)>> msgFuncMap;
};


#endif /* WORKSPACE_MARKET_INTERFACE_TRADEREVENT_TRADEREVENT_H_ */
