/*
 * manageEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INTERFACE_MANAGEEVENT_MANAGEEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_MANAGEEVENT_MANAGEEVENT_H_

#include <functional>
#include <map>
#include <string>

struct MsgStruct;

struct ManageEvent {
  bool init();
  void handle(MsgStruct &msg);
  void regMsgFun();

  std::map<std::string, std::function<void(MsgStruct &msg)>> msgFuncMap;
};

#endif /* WORKSPACE_MARKET_INTERFACE_MANAGEEVENT_MANAGEEVENT_H_ */
