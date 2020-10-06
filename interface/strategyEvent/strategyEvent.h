/*
 * strategyEvent.h
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_STRATEGYEVENT_STRATEGYEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_STRATEGYEVENT_STRATEGYEVENT_H_
#include <map>
#include <functional>
#include <string>
struct MsgStruct;
struct StrategyEvent
{
    bool init();

    void handle(MsgStruct& msg);
    void regMsgFun();

    std::map<std::string, std::function<void(MsgStruct& msg)>> msgFuncMap;

};



#endif /* WORKSPACE_MARKET_INTERFACE_STRATEGYEVENT_STRATEGYEVENT_H_ */
