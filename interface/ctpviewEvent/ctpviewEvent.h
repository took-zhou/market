/*
 * ctpviewEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INTERFACE_CTPVIEWEVENT_INTERACTOREVENT_H_
#define WORKSPACE_MARKET_INTERFACE_CTPVIEWEVENT_INTERACTOREVENT_H_

#include <functional>
#include <map>
#include <string>

struct MsgStruct;

struct CtpviewEvent {
  bool init();
  void handle(MsgStruct &msg);
  void regMsgFun();

  void LoginControlHandle(MsgStruct &msg);
  void CheckStrategyAliveHandle(MsgStruct &msg);
  void BlockControlHandle(MsgStruct &msg);
  void BugInjectionHandle(MsgStruct &msg);

  std::map<std::string, std::function<void(MsgStruct &msg)>> msgFuncMap;
};

#endif /* WORKSPACE_MARKET_INTERFACE_CTPVIEWEVENT_INTERACTOREVENT_H_ */
