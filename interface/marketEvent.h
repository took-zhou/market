/*
 * MarketEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_

#include "market/interface/ctpEvent/ctpEvent.h"
#include "market/interface/ctpviewEvent/ctpviewEvent.h"
#include "market/interface/manageEvent/manageEvent.h"
#include "market/interface/strategyEvent/strategyEvent.h"
#include "market/interface/traderEvent/traderEvent.h"

#include <functional>
#include <map>
#include <string>
#include "common/self/dci/Role.h"

struct MsgStruct;
struct MarketEvent : CtpEvent, StrategyEvent, ManageEvent, TraderEvent, CtpviewEvent {
  MarketEvent(){};
  MarketEvent(const MarketEvent &) = delete;
  MarketEvent &operator=(const MarketEvent &) = delete;
  static MarketEvent &getInstance() {
    static MarketEvent instance;
    return instance;
  }
  bool init();
  bool run();
  void regSessionFunc();
  IMPL_ROLE(CtpEvent);
  IMPL_ROLE(StrategyEvent);
  IMPL_ROLE(ManageEvent);
  IMPL_ROLE(TraderEvent);
  IMPL_ROLE(CtpviewEvent);
  std::map<std::string, std::function<void(MsgStruct msg)>> sessionFuncMap;
};

#endif /* WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_ */
