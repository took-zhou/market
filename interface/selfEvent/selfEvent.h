/*
 * selfEvent.h
 *
 *  Created on: 2020��8��30��
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_SELFEVENT_SELFEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_SELFEVENT_SELFEVENT_H_


#include <map>
#include <functional>
#include <string>
struct MsgStruct;

struct SelfEvent
{
    bool init();
    void handle(MsgStruct& msg);
    void regMsgFun();
    void HeartBeatHandle(MsgStruct& msg);

    std::map<std::string, std::function<void(MsgStruct& msg)>> msgFuncMap;
};

#endif /* WORKSPACE_MARKET_INTERFACE_SELFEVENT_SELFEVENT_H_ */
