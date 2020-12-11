/*
 * ctpEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_CTPEVENT_CTPEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_CTPEVENT_CTPEVENT_H_
#include <map>
#include <functional>
#include <string>
#include "common/extern/ctp/inc/ThostFtdcMdApi.h"

struct MsgStruct;

struct CtpEvent
{
public:
    bool init();

    void handle(MsgStruct& msg);
    void regMsgFun();

    // 处理深度行情数据
    void DeepMarktDataHandle(MsgStruct& msg);

    // 处理登录事件处理
    void LoginInfoHandle(MsgStruct& msg);

    // 处理登出事件处理
    void LogoutInfoHandle(MsgStruct& msg);

    std::map<std::string, std::function<void(MsgStruct& msg)>> msgFuncMap;
private:
    std::string reqInstrumentFrom = "local";
};



#endif /* WORKSPACE_MARKET_INTERFACE_CTPEVENT_CTPEVENT_H_ */
