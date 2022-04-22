/*
 * strategyEvent.h
 *
 *  Created on: 2020.11.13
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

    // 处理策略端请求的合约信息
    void TickSubscribeReqHandle(MsgStruct& msg);

    // 发布tick数据进度控制
    void TickStartStopIndicationHandle(MsgStruct& msg);

    // 策略是否运行回复处理
    void StrategyAliveRspHandle(MsgStruct& msg);

    void TimeLimitReqHandle(MsgStruct& msg);

    std::map<std::string, std::function<void(MsgStruct& msg)>> msgFuncMap;
};

#endif /* WORKSPACE_MARKET_INTERFACE_STRATEGYEVENT_STRATEGYEVENT_H_ */
