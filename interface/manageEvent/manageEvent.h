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
#include "common/self/utils.h"

struct ManageEvent {
  ManageEvent();
  void handle(utils::ItpMsg &msg);
  void regMsgFun();

  void TickMarketStateReqReqHandle(utils::ItpMsg &msg);

  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msgFuncMap;
};

#endif /* WORKSPACE_MARKET_INTERFACE_MANAGEEVENT_MANAGEEVENT_H_ */
