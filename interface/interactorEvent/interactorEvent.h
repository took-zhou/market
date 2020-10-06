/*
 * interactorEvent.h
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_INTERACTOREVENT_INTERACTOREVENT_H_
#define WORKSPACE_MARKET_INTERFACE_INTERACTOREVENT_INTERACTOREVENT_H_


#include <map>
#include <functional>
#include <string>
struct MsgStruct;

struct InteractEvent
{
    bool init();
    void handle(MsgStruct& msg);
    void regMsgFun();

    std::map<std::string, std::function<void(MsgStruct& msg)>> msgFuncMap;
};

#endif /* WORKSPACE_MARKET_INTERFACE_INTERACTOREVENT_INTERACTOREVENT_H_ */
